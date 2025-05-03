## HUI
*HTML UI* \
*Hyper Text Markup Language User Interface* 

Library aiming to deliver simple API for building apps with `HTML`+`CSS`+`JS` UI in `C`/`C++`/`Python`/`Rust` and thus make developing GUI app as easy as it can be.
You will be not limited to desktop apps - HTTP backend lets you to do anything from desktop app to web app with one API (*comming soon; see limitations*).
Currently Linux and Windows are supported *(more comming soon)*.
I'm aware that the code quality is not the best however it mostly works plus it's up to you whether you decide to use it.

<!-- 
There should be as little options as possible while offering wide customizations options.
Why HTML? - its most used (web and some apps), has many features and is easily customizable (with cost of lower performace - but thats not an issue today - BTW on RPi1B it worked usably)
The library should be always backwards compatible (i hope that this will be guarantabe from its v1.0.0, now its not even close to compatibility).
Different from ElectronJS. - backend code is C++ (better performance, easier integration) - HTML and JavaScript should be used only for UI and not for app logic.
All apps should be designed to be usable on any screen size (from workstations to mobile phones or even smartwatches) and with mouse or touch or anything else. 
 -->


### Configuration
<!-- HUI-styling can be really big project -->

#### HUI-theming
I propose idea of having custom user settings for all HTML UIs - having custom  `*.css` and `*.js` file that gets always loaded (and doesnt mess up anything else).
This would be great not only for apps using HTML but also for web pages (only there would be issue that we are used to their visual signature that can protect us from fake sites).
This not only it will looks nice from users perspective, but it will also be much easier to develop nice UI (and thus cost effective and/or less time consuming).

**current status:** \
Default theme is in `HUI_theme.css` and is included within builds (additionally some more tweaks are loaded too).
To apply custom theme, you can use CSS and create custom stylesheet and save it to `$HOME/.config/HUI_theme.css` or `%userprofile%/HUI_theme.css` (just try to not break the layout; you should implement the `.hui_unstyled` class as in the default theme).

#### Other options
For simplicity, all other settings (like language, platform-specific options, ...) are also included in theme file as CSS variables. (Consult the `hui_theme.css` for more info.)
<!-- Lets keep these options in grep'able format so non-HUI software can benefit from it too. -->

<!--#### Shebang for HTML apps
...
Currently TODO, it would be nice to just download html file (or zip of files) and run it like appimage/py script/standalone binary (HUI would be the only dependency)
(there are already frameworks for html/css/js apps)
PWA apps <https://learn.microsoft.com/en-us/windows/android/pwa> <https://developer.mozilla.org/en-US/docs/Web/Progressive_web_apps/Guides/Making_PWAs_installable>
<https://github.com/xtools-at/Android-PWA-Wrapper> <https://stackoverflow.com/questions/76089991/are-pwas-published-to-play-store-installable-without-any-browsers-installed-on-d>
should this be inbuilt into browser?: <https://www.omgubuntu.co.uk/2025/03/firefox-nightly-supports-web-apps-taskbar-tabs>
-->

### API
This time the code is the documentation - look into the code - `HUI.h` and `HUI.hh` should give you some ideas on how it works.
<!-- 
TODO: main place for documentation (language agnostic, based on C api), should be here - with colapsible sections (for either all info or for bugs, language-specific notes, limitations, todos)

Methods returning const char* - caller becomes the owner of the returned string and has the responsibility for (not)freeing it (devs who code in C should really read the documentation).
WebView::window_handle under windows returns HWND = PVOID = void*
WebView::call_js executes js only in main frame (its users responsibility to deal fith iframes + it wouldnt be possible to support iframes in HTTP backend so this feature is not comming any time soon); checked: cef, qt5
WebView::backend_name returns lowercase string of <web rendered>-<toolkit/helper library>
WebView::window_type returns lowercase string with x11/wayland/windows/unknown
Two options: regular HTML5 or HUI-HTML (.hui_unstyled & disabling hui_additions).
WebView::handle_threaded doesnt work (can be implemented by backend that support it).
WindowGeometry - value -1 always means unset.
WindowGeometry::focused can be same as !bool(WindowGeometry::stacking_position) however stacking_position is on most systems imposible to get/set 

	  /*
WindowControls props:
title
*icon ~ TODO: window icon (not supported on most wayland compositors - xdg_toplevel_icon) -- app icons will work trough app implementation (not sure how this works -- windows-app icon/load image; x11?load image; wayland defined somewere in filesystem by app_id)
*status ~ TODO: window progress (windows green progress, request activation, error) -- for wayland likely just edit title with [progress: 50%] [activate/error] and make the panel parse it
id
type
geometry ~ floating=tiling=polyscreen{x,y,w,h,(screen)} / maximized{screen} / fullscreen{screen} / hidden{} -- relative positionning done trough callback (for embeding): 'function(...monitors geometry[]){return new position}'
exclusive_zone
opacity ~ *transparency/backroundblur(never there)
**relative_to/parent/child
**input_mode_{keyboard/mouse/touch/?graphic tablet/?pen/...} VS. non rectangular windows (under wayland wl_surface::set_input_region)
**geometry_constrains (was never implemented, can come with callbacks/relative positions, not needed)

WindowControls other:
drags
closereq (only thing really missing, will need to figure out how to deal with multiwindow apps in api way)

*/



	void start_move_drag (){
		gtk_window_begin_move_drag (GTK_WINDOW(window), NULL, NULL, NULL, NULL);
	}
	
	void start_resize_drag (){ // TODO: select / auto-select resize edge/corner
		gtk_window_begin_resize_drag (GTK_WINDOW(window), GDK_WINDOW_EDGE_SOUTH_EAST, NULL, NULL, NULL, NULL);
	}
	
	// TODO: for compositors like wayfire there should be more start_* (like rotate, scale, opacity ctl, ...) - THIS SHOWS WHY CSD IS BAD IDEA
	
	
	void close () { // TODO: close = destroy = delete variable vs. request close = decorations buttons
		gdk_window_destroy(gtk_widget_get_window(GTK_WIDGET(window)));
	}
	
	
	std::function<void()> on_closing = [this](){
		#if defined (HUI_GTK3_DEBUG)
			std::cout<<"\n [HUI::DEBUG] closing window and exiting ";
		#endif
		close();
		exit();
		};
	
	std::function<void()> on_geometry_changed = [](){}; // TODO: no triggers, 
	
	std::function<void()> on_activated_deactivated = [](){}; // TODO: no triggers, request_activation vs. set_input_keyboard
	
	
	// TODO:  calls: move/resize drag, close=destroy
	// TODO:  callbacks: changed_geometry, activate/deactivate, close_req/window_menu(~titlebar_buttons), drag/drop (NO - will be handled in js), cut/copy/paste (NO - will be handled in js)


	
		// TODO: exclusive zone


	// TODO: activation
	
	
	// TODO: move/resize drag
	
	void close () {
		(*window)->close();
	}
	
	
	// TODO: on_close - needs overriding qt method <https://stackoverflow.com/questions/17480984/how-do-i-handle-the-event-of-the-user-pressing-the-x-close-button> 
	
	// TODO: on_geometry, on_activate
	
	
	
-->


### Building apps
*I decided to stick with the simplest tools.*

Linux or MinGW `g++` (C++):
```
cd build
g++ -o <binary> ../<source> -I../../HUI -L. -lHUI
```
Linux or MinGW `gcc` (C):
```
cd build
gcc -o <binary> ../<source> -I../../HUI -L. -lHUI
```
Windows MSVC `cl.exe` (C++):
```
cd build
cl /EHsc /Fe:<binary>.exe ../<source> /I../../HUI /link /LIBPATH:. libHUI.lib
```
Windows MSVC `cl.exe` (C):
```
cd build
cl /Fe:<binary>.exe ../<source> /I../../HUI /link /LIBPATH:. libHUI.lib
```
Rust (first, copy your project into HUI root):
```
cd <your project>
cargo build --release
```

(All examples assume that HUI repo and app repo directories share the same directory. And that `build` directory already exists and contains `libHUI.so` or `libHUI.dll + libHUI.lib`)
<!-- You may want to use $LD_LIBRARY_PATH or %PATH% environs if you dont have HUI installed or if you are experimenting with multiple versions. -->

<!--
Additionaly you can include HUI into your project as header-only library - generally by adding `#include "hui_<backend name>.cc"` after `#include "HUI.hh"` and changing the build command a little. \
Build HUI with your project: ``g++ -o <your_executable> <your_source> `pkg-config --cflags --libs whatever` ``
 -->
It is also possible to use HUI as header-only library (just include the backend file `hui_*.cc` before including `HUI.hh`) - then you have to link to libraries that libHUI links to (generally, you just modify the command that is used to build libHUI to build binary and add libraries you use).


### Building HUI

#### Shared library
**The recommended way is to use build scripts (files that start with `build_`).** \
Build scripts can also serve as documentation for the build process (I really recommend reading them before running them). \
To start build, run: \ 
`build_<backend>.{sh,bat,py} [--dependencies] [--deploy] [--tests]`
<!--
--dependencies ... install dependencies
--deploy ... make the build directory self-contained
--tests ... build tests
---
--debug ... debug define
--build ... happens alyways (should it?)
--python ... python bindings
 
**building HUI as shared library (resulting in libHUI.so/libHUI.dll + dependencies)**
-->


#### Python module/bindings
Python module is build just like any other app using HUI (except that here you build dynamic library instead of binary). \
The bindings are defined in `hui_python.cc` and use [pybind11](https://pybind11.readthedocs.io/en/stable/compiling.html#building-manually).
First, you have to build libHUI (you can use dummy build as template for linking, provided that you used same compiler), then run `build_python.py` (again, this file can serve as documentation).


### Developement

#### Current status
HUI 'newnew' - attempt to reimplement HUI from beginning \
*(few things are yet to be done to have usefull library - styling, bugs, hui init, content loading, js api)*
| BACKEND | STATUS | OS | NOTES |
|---|---|---|---|
| WebKit (GTK3) | *working* | Linux (x86_64, arm64, armhf) | ok |
| Blink (CEF) | *working* | Windows (AMD64) | GPU acceleration disabled (causes crashes); transparency not working |
| Webkit (Qt5) | *working, deprecated* | Windows, Linux | uses old HTML/CSS/JS |
| none (dummy) | *working* | *all* |  |



#### TODOs / Progress tracking
<!-- sort from most important to least -->

<details><summary><h3> important </h3></summary>

- [ ] clean exit -> exit()
- [ ] cpp: fix copy/move constructor issues with HUI::WebView()
- [ ] rust bindings: clean + polish + set_geometry + constants

</details>


<details><summary><h3> building </h3></summary>

- [ ] single master build script for everything + make it possible to combine windowcontrols/webview/windowembeder in different languages at linkage time
- [ ] debug control flag/rust --debug/release
- [ ] c, cpp, rust, python bindings in own directories
- [ ] put backends in own directories + make it possible to build webview and controls separately
- [ ] handle hui.js and hui.css as define/text replace (so we dont need quoting)
- [ ] create master build script that calls others + build scripts directory + release naming ( HUI-{platform}-{backend}_{version or commit hash}_{more info} )
- [x] ~~upgrade build scripts (error handling, bindings, packaging, colors) -> multibuild script calling the other scripts~~

</details>


<details><summary><h3> backends </h3></summary>

- [ ] js bindings: expose whole api to js (just set of callbacks from js)
- [ ] rust http server (something between desktop app and web app) + url or browser in kiosk mode <https://superuser.com/questions/716426/running-latest-chrome-for-windows-in-kiosk-mode>
- [ ] new backend: rust HTTP backend (something between desktop app and web app => "server mode" - app can run on headless server; prints out ui url / opens browser in kiosk mode ~ can work with with wrapper to get window id to control it) ~ multiuser: configurable action when user tries to connect existing instance (new app instance OR nothing; should be per app or per backend configurable?)
- [ ] cef linux (rework current cef impl)
- [ ] support MacOS - should work but untested + missing build scripts <https://ports.macports.org/port/webkit2-gtk/details/>
- [ ] consider more backends, even proprietary <https://ultralig.ht> <https://docs.rs/sciter-rs/latest/sciter/> (sciter is probably not compatible with html5/css3)
- [ ] update Qt backend with all Qt webViews availible (WebKit, CefView, MozEmbed, Ultralight, ...) - <https://github.com/niutech/qt-ultralight-browser?tab=readme-ov-file> -- this seems to be the easiest way to support the most backends
- [ ] new backend: gtk4/5 rust webkit
- [ ] console backends: Cuervo (https://servo.org/made-with/#), the terminal browser project with chromium, old console browsers with js support

</details>


<details><summary><h3> js api </h3></summary>

- [ ] 'js_object'
- [ ] use 'app://' custom protocol for loading files/content
- [ ] more native and faster 'html_element'/js api
- [ ] support js types (instead of just js code as string)
- [ ] return from callback

</details>


<details><summary><h3> tests </h3></summary>

- [ ] create example_simple and example_advanced in all supported languages that will to exactly the same
- [ ] proper tests: async js, window ctls, load_*

</details>


<details><summary><h3> api </h3></summary>

- [ ] support drag and drop and copy/paste for text, files, images, application-specific (=allow real paths + predefined callbacks) -> define for events in call_native
- [ ] callbacks for window close (on_closing vs. on_closed)
- [ ] callbacks for window controls
- [ ] reconsider window.show/realize
- [ ] add concept of app - name, icon, ...  => app property: void set_taskbar_icon(icon, name) + app.add(HUI::Window)
- [ ] threaded/multiprocess message loop (will need to figure out a way to return thread to c api) + thread-safety (the easiest solution seems to be creating optional thread safe wrapper running everything in external thread and handling the communication)

</details>


<details><summary><h3> tweaks </h3></summary>

- [ ] support client-side decorations trough user styling/scripting  + select/get decoration scheme
- [ ] auto update theme (not sure about the js) -- maybe it can be just autorefresh implemented in js for loading stylesheet from custom app:// protocol
- [ ] get rid of private js and css (if possible and if safe) -- it should work as simple inject css/js as in browsers (limited to app:// protocol)
- [ ] (BIG PLAN) allow dynamically changed themes per screen/app/schedule *.css (may be done trough external tool and storage unshare -> HUI wayfire tool ~ but how to multiwindow?) --- per screen themes (TODO: need to be able to have two themes aplied side by side when shown on more than one monitor ~ switch themes quickly and stop updating the other area of the window)
- [ ] have option to run like regular webview -> rethink the automatic running of `hui_tweaks()` 

</details>


<details><summary><h3> future </h3></summary>
 
- [api] (BIG PLAN) upgrade API to serve as fully-featured and platform/renderer-agnostic base for building web browser (browser-beta)

- [x] add back windows controls; controls can have dual handles - backend + handle - or there may be 2 types of controls (toolkit + os) == window handle X backend pointer X c api pointer
- [ ] WindowEmbeder (?): fix: black space when resizing -> two options = black artifacts and instant resize OR little laggy resize but no artifacts; [window embeding (see win implementation and wayland notes in old readme)]
- [ ] window controls: parent/child windows (see old readme); popup windows (likely just prepared regular window; positioning and window type is yet to be discussed especialy for wayland)
- [ ] wayland controls: <https://wayland.app/protocols/xdg-shell>

- [ ] check for memory leaks - install valgrind <https://valgrind.org/docs/manual/quick-start.html#quick-start.intro> <https://valgrind.org/downloads/current.html> <https://valgrind.org/docs/manual/dist.readme.html>

- [ ] refine purpose of hui.js/hui.css/theme file (fr scale + colors + elements) -- complete user styling and settings (1 css file)
- [ ] unify js helper names across all platforms

- [ ] ?better api (or just leave it for now):  load_uri ;;; (string map based settings -- browser specific -- similar to flags) ;;; call_native handled by lang bindings -> simpler, easier, safer (but more complicated) -- make the call_native() be handled by the language bindings (issue with c->cpp, c only, header-only)
- [ ] run without window manager - see notes in old readme (its better to include tiny wl compositor ~ so maybe nothing has to be done here)
- [ ] allow moving running app from one browser to another (restore html and js context) -- without action from dev code (CANT BE THAT HARD - serialize all js including functions + <html>.outerHTML)
- [ ] (BIG PLAN) display server in browser (replacement for x11 and wayland -- when launched in browser in kiosk mode as fullscreenm) - relatively easy + can pair with greenfield (html wayland compositor -> support for legacy/direct drawing)

- [ ] add popup (any popup ~ popups are not currently supported) that can exist outside the window (may be good idea to prepare the popup window)

- [ ] research existing projects/ideas that can be used within this project

- [ ] ? put sent2cpp_handlers into impl (if possible) or get rid of them; do: js calls cpp -> cpp calls one function that handles callbacks on the api/abi level === store native callbacks so that we can skip the C++ to C to C++ (maybe - seems to be complicated in respect to its benefits) -- will be likely better than the hell when converting the callbacks from one language to another
- [ ] ? inheritance for backend implementations (and maybe namespaces for backends -> multi-backend); methods: call_native, call_js/call_js_async, html_element
- [ ] ? controls: callbacks

</details>




 

 
 
#### BUGs:
 - [api] C++ api wrappers likely contain memory leaks (losing reference) + copy/move is not properly handled
 - [backend gtk3] WEBKIT_DISABLE_COMPOSITING_MODE=1 + the transparency became random (vbox + 3d disabled; 3d enabled is buggy); similar: <https://github.com/tauri-apps/tauri/issues/8254>
 - [backend gtk3] hover effect doesnt reset when mouse exits window at the same time as exiting window
 - [backend gtk3] window_handle returns pointer to wl_surface (better option should be considered ~ [XDG-foreign](https://wayland.app/protocols/xdg-foreign-unstable-v2#zxdg_imported_v2) if it will work for what we want)
 - [tests] test if call_js return works for multiple statements
 - [backend cef] calling destructor of webview causes crash instead of just closing the window
 - [backend cef] window doesnt close when it goes out of scope
 
 
#### Developement rules
 - keep everything simple
 - dont change things that do work 
 - WebView, WindowControls, WindowEmbedder should be designed and developed independently - merging these can be done externaly (inheritance/member) and we can keep the API modular (old version can be kept while introducing new one)
 - with any new API design, start with C
 - provide build scripts and docs
 - naming convension: `hui_*` - backend code, `HUI*` - API code
 - every script should be ran from repo root = all commands should start and end in root of repo
 - readme should go from less developer to more developer (from project about over api docs to developement informations; "end user", "app developer = library user", "library developer")
 - readme should be only one, otherwise its a mess

<!--
Can "C to C++" be treated as backend? - No, unless we completely skip the C part.
But, in theory, for each language bindings, you choose from: {C to your language, ... (any backend that is written in that language)}

What exactly is browser script / user style vs. our css theme / js additions ? 

html title atribute - for accesibility (voice controll/...) <https://stackoverflow.com/questions/7503183/what-is-the-easiest-way-to-create-an-html-mouseover-tool-tip>
focusable item = item controlable in TUI mode (likely this will create invisible window and use js to obtain element position) <https://stackoverflow.com/questions/32483752/window-vs-offscreen-window> <https://stackoverflow.com/questions/442404/retrieve-the-position-x-y-of-an-html-element>
implement embed windows (including detachable embeds) -- there may exist more types of content: webview/embed/pixmap/detachable embed (the root/main window should be blank/transparent)

 <details><summary>research existing projects/ideas that can be used within this project</summary>

<https://github.com/tauri-apps/wry> - more about app distribution + js apps
<https://github.com/webview/webview/tree/master> - less backends supported than planned for HUI
<https://github.com/microsoft/playwright> - this is the state of backend support we want

</details>

<details><summary>implement parent/child windows = implement popup dialogs blocking access to main window</summary>

Should this be really implemented? Looks like it is something against user experience. Are there any situations when this is needed? (close window dialog, file chooser)
Child windows are represented in the panel together with their main windows by one entry. 
The hierarch can have multiple levels. (main window -> save file -> file exists)
[wayland-only activation](https://stackoverflow.com/questions/75861687/how-to-request-a-window-be-activated-with-the-wayland-protocol)
OR this can be easily achieved without this library - just activate child window when main activated + disable body using html 

</details>

<details><summary>separate the webview from the window and add option to prepare window</summary>

Embeded windows on Wayland can be a [problem](https://bugzilla.gnome.org/show_bug.cgi?id=721224). - so instead of GtkPlug/GtkSocket, relative windows should be always used
[x11 parent window](https://unix.stackexchange.com/questions/197188/can-i-put-multiple-x-windows-into-a-single-larger-window)
[wayland book](https://wayland-book.com/xdg-shell-basics/xdg-toplevel.html)

</details>

<details><summary>run without window manager (linux only)</summary>

for starters see <https://stackoverflow.com/questions/21990111/why-qt-can-run-without-x-server-and-gtk-cant>
or better just use tiny wayland compositor and have single window inside (like kiosk, example: <https://github.com/swaywm/wlroots/blob/master/tinywl/tinywl.c>)
maybe due to window embeding capability, the compositor will be needed even for normal mode (this would allow skiping x11 embeding)
offscreen rendering: <https://github.com/daktronics/cef-mixer>

</details>

backends ideas:
 * Qt: WebKit, [Chromium](https://cefview.github.io/QCefView/), [Gec](https://github.com/sailfishos/qtmozembed)[ko](https://stackoverflow.com/questions/67711180/how-to-use-gecko-with-c-and-qt)
 * GTK4 WebKit2
 * Webkit2
 * Servo ([libsimpleservo](https://www.reddit.com/r/servo/comments/940n14/libservo/))
 * Chromium (CEF)
 * w3m/elinks (~console browser -> TUI)
 * http/https webserver


-->


<!--
---

<!-.-  -.-> 

<details><summary><h3> section </h3></summary>

content

</details>

---
--> 

<!--
### Docs
*not much documentation yet - this time the code is the documentation - look into the code - `HUI.h` and `HUI.hh` and `build_*.{bat,sh}*`

### API
API NOT FULLY DOCUMENTED YET - LOOK INTO THE CODE - CURRENTLY FOCUSING ON DEVELOPEMENT

#### `HUI::Str`
 - string implementation

#### `HUI::Vec`
 - vector implementation

#### `HUI::Window`
 - class for simplifying the process of creating window - creates a window with HTML view inside and exposes window controls
 - inherits methods from `HTUI::WindowBase` and `HTUI::WindowControls` and  `HTUI::WindowContent` (puts all the functions for the UI into a single class)
 
#### `HUI::WindowContent` = `HTUI::WindowContentHTML`
 - class of the webview
 
	<!-.- - members (inherit): -.->
 
	##### `void content (HUI::Str filename)`
	 - sets HTML from file
	 * there should never happen page reload - the js context should remain same + all windows should have it

	##### `HUI::Str call_js (HUI::Str code, bool return_data=false)`
	 - evaluates js from given string
	 - returns either the result or nothing depending on the second argument 
	 - only `return_data=false` is guaranted to work before and after calling handle (in rare cases ce accidentally be run twice), `return_data=true` is inteded to be used in callback functions
	 * **TODO** add run on finsh return (async)

	##### `HUI::Str call_cpp (std::function<void(std::vector<std::string>)> function)`
	 - exposes cpp function to js
	 - returns js code to call the function with 

	##### `void* backend_widget ()`
	 - provides access to the backend object
	 
	<!-.- - members (other): **TODO** -.->
	
    ##### `void js_set_value (const char* code, auto value)`
	 - possible types of value: `HUI::Str`/`long`/`std::function<void(std::vector<std::string>)>`
	 
	##### `void js_set_value (const char* query, const char* property, auto value, uint16_t index = -1)`
	 - similar to previous but sets property of all elements selected by `document.querySelectorAll`
	 
	##### `HUI::Str js_get_value (const char* code)` and `HUI::Str js_get_value (const char* query, const char* property, uint16_t index = 0)`
     - returns value of js variable/property of element converted to string
 
#### `HUI::WindowBase`
 - class of the window = root container
	<!-.- - members (inherit): -.->
 
	##### `void add (HUI::WindowContent* content)`
	 - adds content to window

	##### `void show ()`
	 - makes the window show up (usefull for situations when you want to initialize the window but show it to the user later or if you need offscreen window)

	##### `void* backend_window ()`
	 - provides access to the backend object
		
	<!-.- - members (other): **NONE** -.->

#### `HUI::WindowControls`
 - class with window controls

	##### `void set_title(const char* title)`
	
	##### `const char* get_title()`
	
	##### `void set_unmanaged ()`
	 - should be called before `HUI::Window::show()`
	 - makes the window frameless and not shown in window list
	
	##### `void set_geometry (WindowGeometry::<type> geometry)`
	 - set window position and size (includes fullscreen and minimize)
	 - `window.set_geometry({.x = -1, .y = -1, .w = 500, .h = 500});`
	 - types: 
		- see definitions in `hui_window_types.h`
	
	##### `WindowGeometry::<type> get_geometry (WindowGeometry::<type>)`
	 - you have to specify return type in the first argument `auto geometry = window.get_geometry(HUI::WindowGeometry::Floating());`
	
	##### `void set_layer(WindowLayer layer)` and `WindowLayer get_layer()`
	 - set/get window layer (wayland layer-shell) or window always on top/bottom
		
	##### `void set_input_mode_keyboard (WindowInputMode mode)` `WindowInputMode get_input_mode_keyboard ()`
	 - set/get whether window is focusable = takes focus = takes input from keyboard

	##### `void set_exclusive_zone (bool enabled, int16_t custom=-1)`
	 - sets the window to not being ovelaped by other windows (only wayland layer-shell)
	
	##### `void close ()`
	 - destroy the window
		
	##### `std::function<void()> on_closing`
	 - change the value to handle app close 
	
	##### ``

-->


<!--

[qt5 docs (almost all you need)](https://doc.qt.io/qt-5/qwidget.html)
[the reason why this should be browser base library (people asking for certain browsers)](https://www.reddit.com/r/browsers/comments/o43647/why_cant_someone_make_a_browser_based_in_webkit/)
[one more reason for multibrowser](https://news.ycombinator.com/item?id=40845951)
[discussion on gui libraries](https://news.ycombinator.com/item?id=39970803)
[how many config files do regular gui toolkits have?](https://www.reddit.com/r/linuxquestions/comments/lmrth9/removing_gtk_window_controls/)
[is there something as window id on wayland?](https://unix.stackexchange.com/questions/362731/how-to-identify-window-by-clicking-in-wayland)
[is this the way for HUI-styling?](https://superuser.com/questions/1684707/how-to-enable-one-line-ui-on-chromium-based-browsers) 

# Notes (old):
	
		- embed window electronjs <https://github.com/electron/electron/issues/10547> 
		- type hint should not exist ([specification](https://specifications.freedesktop.org/wm-spec/wm-spec-1.4.html), [gtk](https://docs.gtk.org/gdk3/enum.WindowTypeHint.html), [work only under x11](https://discourse.gnome.org/t/gtk3-with-wayland-bugs-using-windowtypehint-desktop-set-keep-below-and-glarea/6100))
		- layer window can be also closed (KDE -> Alt+F3 -> close)
		- external hui methods:  tray icon (...), notifications (...), ... [??? custom panel and pager hint]


	## TODOs:
		- GTK3:
			* drag n drop (<https://developer.mozilla.org/en-US/docs/Web/API/HTML_Drag_and_Drop_API>, <https://www.w3schools.com/html/tryit.asp?filename=tryhtml5_draganddrop>)
			* bubbles/popups (=prepared hidden window - or maybe webview will be enough)
			* add client/(server) decorations = add api for creating them (close, maximize toggle, mimize toggle, drag region, ... -> <https://docs.gtk.org/gtk3/method.Window.set_titlebar.html>)
			* ?option to disable all htui aditions like htui.js -> then just use embed web renderer window
			* ?load page from string (for safety; or maybe not cos u cant access any external file)
			* window hierarchy:  [no parent_window ; no is_in_panel] - main_window / child_window = is_in_panel - not needed - all "normal" windows should be in panel - and we dont need windows blocking
			* type hint: gdk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DOCK/GDK_WINDOW_TYPE_HINT_NORMAL/GDK_WINDOW_TYPE_HINT_DESKTOP);
			* embed (<https://github.com/endlessm/maxwell>)
			* ? can optimize size by redefining gtypes (<https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Basic-Types.html>)
			* layer window mouse/kbd events - yes/no/exclusive/copy(only for mouse hover) - look at <https://shallowsky.com/blog/programming/translucent-window-click-thru.html> <https://dev.tizen.narkive.com/AkKHZRHh/wayland-non-rectangular-wayland-window-input-through> <https://docs.gtk.org/gtk3/method.Overlay.set_overlay_pass_through.html>
			* switch between configs (normal window, layer window, window managing --or convert them to separate functions) - layer vs normal window: to switch, create new one
			
		- QT:
			* qt multiwebview implementation ([chromium](https://cefview.github.io/QCefView/), [gecko](https://github.com/sailfishos/qtmozembed, https://stackoverflow.com/questions/67711180/how-to-use-gecko-with-c-and-qt), [webkit/webkit2](https://doc.qt.io/archives/qt-5.5/qtwebkitexamples-index.html))
			* qt layer shell <https://invent.kde.org/plasma/layer-shell-qt/-/blob/master/tests/main.cpp?ref_type=heads> <https://www.reddit.com/r/QtFramework/comments/wqky3o/qt6_and_wlrlayershell/>
			* embed <https://stackoverflow.com/questions/36035983/how-can-i-embed-a-custom-qwidget-in-a-qwebengineview-or-page>
			
		- etc...
			* single app should use less than 100MB RAM
			* native file selector (use default file manager)
			* window covering all screens (just one big window thats placed well enough to cover all screens)
			* for CEF: <https://www.electronjs.org/docs/latest/api/webview-tag>
			* similar project: <https://github.com/webview/webview/tree/master>
			
		- hui.js:
			* disable zooming/scrolling/selection (will be done in htui.js)
			* <https://oozou.com/til/create-element-with-attributes-in-one-line-69>
			* <https://stackoverflow.com/questions/574941/best-way-to-track-onchange-as-you-type-in-input-type-text>


	## BUGs:
		- GTK3:
			* frameless not working on kde wl
			* set screen not working on kde wl
			* background layer on kde wl


	## Resources:
		- HUI::Str:
			* <https://stackoverflow.com/questions/20997838/inheritance-selecting-which-base-class-methods-to-inherit>
			* <https://en.cppreference.com/w/cpp/language/operators>
			* <https://cplusplus.com/reference/string/string/>
			* <https://stackoverflow.com/questions/16504062/how-to-make-the-for-each-loop-function-in-c-work-with-a-custom-class>
			* <https://stackoverflow.com/questions/442026/function-overloading-by-return-type>
			* <https://cplusplus.com/doc/tutorial/inheritance/>
			* <https://stackoverflow.com/questions/14184341/c-constructor-destructor-inheritance>
			* <https://stackoverflow.com/questions/8093882/using-c-base-class-constructors>
			* <https://stackoverflow.com/questions/59670/how-to-get-rid-of-deprecated-conversion-from-string-constant-to-char-warnin>
			* <https://stackoverflow.com/questions/26963510/error-passing-const-as-this-argument-of-discards-qualifiers>
			* <https://stackoverflow.com/questions/50026430/modern-c-array-arguments>
			* <https://stackoverflow.com/questions/4214314/get-a-substring-of-a-char>
			* <https://stackoverflow.com/questions/17591624/understanding-the-source-code-of-memcpy>
			* <https://stackoverflow.com/questions/2242498/c-memcpy-in-reverse>
			* <https://stackoverflow.com/questions/19891962/c-how-to-append-a-char-to-char>
			* <https://stackoverflow.com/a/21146999>


		- GTK3:
			* <https://github.com/wmww/gtk-layer-shell/tree/master>
			* <https://wmww.github.io/gtk-layer-shell/gtk-layer-shell-Gtk-Layer-Shell.html#gtk-layer-set-exclusive-zone>
			* <https://github.com/swaywm/wlr-protocols/blob/master/unstable/wlr-layer-shell-unstable-v1.xml>
			
			* <https://docs.gtk.org/gdk3/enum.WindowTypeHint.html>
			
			* <https://stackoverflow.com/questions/44425758/need-to-call-gtk-init-function-but-not-from-main-function-c>
			* <https://book.huihoo.com/gtk+-gnome-application-development/sec-mainloop.html>


			* <https://stackoverflow.com/questions/36994927/gtk3-window-transparent>
			* <https://stackoverflow.com/questions/46568169/is-it-possible-to-make-a-webkit-webview-transparent-within-the-application-like>
			* <https://webkitgtk.org/reference/webkit2gtk/unstable/method.WebView.set_background_color.html>
			
			* <https://www.reddit.com/r/rust/comments/p79nlm/rust_webkit2gtk_ping_pong_example/>
			* <https://webkitgtk.org/reference/webkit2gtk/unstable/method.UserContentManager.register_script_message_handler.html>
			* <https://docs.huihoo.com/gtk/tutorial/2.0/x159.html>
			* <https://stackoverflow.com/questions/49638121/gtk-g-signal-connect-and-c-lambda-results-in-invalid-cast-errors>

			* <https://webkitgtk.org/reference/webkitgtk/stable/webkitgtk-webkitwebview.html#WebKitWebView-load-finished>
			* <https://webkitgtk.org/reference/webkit2gtk/unstable/method.UserContentManager.add_script.html>
			
			
			* <https://stackoverflow.com/questions/21928654/use-c11-lambdas-as-callbacks-in-gobject-library>
			* <https://stackoverflow.com/questions/22384333/passing-additional-arguments-to-gtk-function>
			* <https://bbs.archlinux.org/viewtopic.php?id=94903>
			
			
		- Qt 5/6:
			* <https://github.com/qt/qtwebview/blob/5.4/examples/webview/minibrowser/main.cpp>
			* <https://doc.qt.io/qt-5/qwebengineview.html#details>
			* <https://packages.msys2.org/package/mingw-w64-x86_64-qtwebkit>
			* <https://code.qt.io/cgit/qt/qtwebview.git/tree/examples/webview/minibrowser?h=6.6>
			* <https://doc.qt.io/qt-6/qml-qtwebview-webview.html#details>
			* <https://www.bogotobogo.com/Qt/Qt5_WebKit_WebView_WebBrowser_QtCreator.php>
			* <https://evileg.com/en/post/110/>
			* <https://doc.qt.io/qt-6/qt.html#WindowType-enum>
			* <https://forum.qt.io/topic/8695/qt-on-windows-avoid-the-taskbar-icon/10>
			* <https://doc.qt.io/qt-6/qtwidgets-widgets-windowflags-example.html>
			
			* <https://doc.qt.io/qt-6/qmake-tutorial.html>
			* <https://forum.qt.io/topic/22882/solved-qapplication-no-such-file-or-directory/3>
			* <https://www.qtcentre.org/threads/67046-warning-invalid-use-of-incomplete-type-class-QEvent>
			
			* <https://wiki.qt.io/Qt_for_Beginners>
			* <https://doc.qt.io/qt-6/qtwidgets-tutorials-widgets-toplevel-example.html>
			* <https://stackoverflow.com/questions/4214369/how-to-convert-qstring-to-stdstring>
			* <https://stackoverflow.com/questions/45481362/set-parent-qwidget-for-promoted-widgets>
			* <https://stackoverflow.com/questions/22959752/qdebug-and-cout-dont-work>
			* <https://stackoverflow.com/questions/43150506/opacity-not-working-with-qwebengineview-and-translucent-background>
			* <https://forum.qt.io/topic/120607/transparent-window-on-both-mac-and-windows>
			* <https://doc.qt.io/qt-6/signalsandslots.html>
			* <https://stackoverflow.com/questions/18975734/how-can-i-find-the-screen-desktop-size-in-qt-so-i-can-display-a-desktop-notific>
			
			* <https://wiki.qt.io/Open_Web_Page_in_QWebView>
			* <https://qtwebkit.github.io/doc/qtwebkit/qwebview.html>
			* <https://het.as.utexas.edu/HET/Software/html/qwebview-members.html>
			* <https://qtwebkit.github.io/doc/qtwebkit/qwebsettings.html>
			* <https://forum.qt.io/topic/29336/qwebview-check-on-valid-load/6>
			* <https://www.qtcentre.org/threads/15594-Disable-context-menu-in-QWebView>
			
			* <https://qtwebkit.github.io/doc/qtwebkit/qtwebkit-bridge.html>
			* <https://stackoverflow.com/questions/19469295/calling-a-qt-function-from-the-javascript-side-qwebview>
			* <https://stackoverflow.com/questions/15646034/cant-build-qt-example-with-q-object-can-i-avoid-qmake>
			* <https://stackoverflow.com/questions/14010922/qt-undefined-reference-to-vtable>
			
			* <https://code.qt.io/cgit/qt/qtbase.git/tree/src/corelib/kernel/qcoreapplication.cpp>
			* <https://code.qt.io/cgit/qt/qtbase.git/tree/src/widgets/kernel/qapplication.h>
			
			* <https://doc.qt.io/qt-6/qwidget.html#visible-prop>
			
		- hui.js:
			- custom elements:
				* <https://developer.mozilla.org/en-US/docs/Web/HTML/Global_attributes>
				* <https://web.archive.org/web/20140118032429/http://www.html5rocks.com/en/tutorials/webcomponents/customelements/>
				* <https://developer.mozilla.org/en-US/docs/Web/API/Web_components/Using_custom_elements>
				* <https://usefulangle.com/post/371/custom-element-css>
				
			- disable zoom:
				* <https://stackoverflow.com/a/60516252/21213243>
				* <https://stackoverflow.com/questions/4472891/how-can-i-disable-zoom-on-a-mobile-web-page>
				
			- disable scrolling/scrollbars
				* <https://stackoverflow.com/questions/51033482/make-entire-page-fit-the-viewport>
				* <https://stackoverflow.com/questions/28411499/disable-scrolling-on-body>
				* <https://www.w3schools.com/howto/howto_css_hide_scrollbars.asp>
				
				

-->
