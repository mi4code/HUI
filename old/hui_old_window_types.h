#if !defined(HUI_WINDOW_TYPES_H)
 #define HUI_WINDOW_TYPES_H

 #include <stdint.h>

namespace HUI {

typedef int16_t WindowPixels; // = 4x 8K screen = 32K screen  // pixels after fractional scale (not real pixels)
typedef int8_t WindowScreen; // = 127 screens

typedef int16_t Pixels; // = 4x 8K screen = 32K screen  // pixels after fractional scale (not real pixels)
typedef int8_t Screen; // = 127 monitors (number) + full workspace = screen (all monitors)

//const Screen S_ALL = -10; // -10~all -1~primary // NOT SUPPORTED YET  // issue with wayfire where layer window is shown only on 1 screen at time 

//Pixels:
//define DONT_SET
//define UNSET -1

//DEFAULT = -1 (~better not -1 == 65535) = dont config ...should exist for all config options (but still with some exceptions)   --there shouldnt be such thing as default not saying what will happen BUT we sometimes need to config only some settings



enum WindowLayer { 

// normal windows (aliases):
  //WL_BACKGROUND =0, // [wm hint] - only unmanaged
	WL_TOPLEAST   =1,
  //WL_TOP        =2,
	WL_TOPMOST    =3,
	
// layer shell windows (values match the layer shell specifications):
	WL_BACKGROUND =0, // background (only this remains shown after user clicks "show desktop")
	WL_BOTTOM     =1, // under windows, over background
	WL_TOP        =2, // over all windows except fullscreen
	WL_OVERLAY    =3, // over everything including fullscreen
	
	//WL_DEFAULT=2     // shouldnt exist (the window must be somewhere -> same as WL_TOP)  --and hidden cant be part of this because after unhide the window should be shown somewhere (same layer as before hide)
    //WL_UNSET=-2       // todo

// note: window layer VS. focused/minimize -- user doesnt change layer (usually) BUT user activates/minimizes the window

};


class WindowGeometry { // TODO: there may be multiple valid states at time, should be used like: set_geometry(WindowGeometry::<window state>(..optional arguments..)) --> no options for WindowGeometry::Floating() will restore previous floating position+size (same for fullscreen/unfullscreen,maximize/unmaximize,minimize/unminimize -> after unset there is previous state)
  public:
	
	struct Anchors { // dimensions are not min_size=size_request - they are only applyed when window dimensions are not defined by the anchors 
		Screen screen = -1;  
		
		Pixels top = -1; 
		Pixels left = -1; 
		Pixels right = -1; 
		Pixels bottom = -1;  
		
		Pixels w = -1; 
		Pixels h = -1; 
	};
	
	/*struct Tiling { // auto-converted to anchors ; minimal definition: 2 edges = corner / 3 edges / 4 edges -> fullscreen = maximized
		Screen screen = -1;  
		
		bool top = false; 
		bool left = false; 
		bool right = false; 
		bool bottom = false;  
		
		Pixels w = -1; 
		Pixels h = -1; 
	};*/
	
	typedef Anchors Tiling;
	
	struct Position { // 0,0 is always upper left corner // TODO: what about "ref_edge = ..." + what about negative position (when is window moved out of the screen and visible only partialy) + wayland relative positioned surfaces
		//Screen screen = -1; // if you want anchors, just use Anchors
	
		Pixels x = -1; 
		Pixels y = -1;  
		
		Pixels w = -1; 
		Pixels h = -1;  

	};
	
	typedef Position Floating;
	
	/*struct FloatingFull { 
		Pixels x = -1; 
		Pixels y = -1;  
		
		Pixels w = -1; 
		Pixels h = -1;  
		
		Pixels w_min = -1; 
		Pixels h_min = -1;  
		
		Pixels w_max = -1; 
		Pixels h_max = -1; 
	}; */
	
	struct Fullscreen {
		bool fullscreen = true;
		Screen screen = -1;
	};
	
	struct Maximized {
		bool maximized = true;
		Screen screen = -1;
	};
	
	struct Minimized { // in fact has nothing to do with geometry of toplevel windows (minimized OR (fullscreen OR (maximized OR tiled/position))), but layer shell windows will be hidden by resizing them
		bool minimized = true;
	};
	
	//typedef Minimized Hidden;
	
};


enum WindowInputMode { // TODO: complete for mouse input
	WIM_ALWAYS,      // keyboard: the only existyng window you can type in              // mouse: default for mouse
	WIM_AUTO_WINDOW, // keyboard: default for keyboard                                  // mouse: is same as _ALWAYS
	//WIM_AUTO_ELEMENT,  // TODO:  keyboard: window focused only when text input element clicked +  mouse: click-trough areas 
	WIM_NEVER,       // keyboard: the window cant get focused=activated so you cant type into it  // mouse: TODO: click trough (better option) OR ignore click (can be done with css)
	
	/*
	
	// OLD: alternative names + functions
	 only-focused
	 only-unfocused
	 autofocus-focused/unfocused [two options: for window / for widgets]
	 manualfocus-focused/unfocused [this way implemented widget focus] - doesnt work is either focused or unfocused
	
	// OLD: WindowEvent enum
	 WE_TAKE, // default - window takes all events (when active)
	 WE_PASS_THROUGH, // dont activate and pass events to underlaying window
	 WE_DUPLICATE, // events go to the window but also are copied to underlaying window
	 WE_EXCLUSIVE // ?hanled by the user - allow click on selected elements / activate window only when user clicked into the input area
	 //WE_BLOCK // take events but act like they never happened (can be done trough html)
     //manual/auto selection of clickable/focusable elements
	
	// OLD: window shapes - not implemented yet (only for LayerWindow - windows manager should handle window shapes themselves + rectangle is best option anyway)
	
	*/
};



enum WindowDecorations { // currently unused // this library is server side decorations only for now + should be decided by wm not by the application // TODO: implement <https://docs.gtk.org/gtk3/method.Window.set_titlebar.html>
	//WD_YES  // decoration scheme should be decided by the compositor
	WD_DEFAULT, // should be yes
	
	WD_SERVER, // titlebar present + frame present // can use gtk-nocsd project or GTK_CSD=0 environ
	WD_CLIENT, // no titlebar + frame present // is ready for custom decorations - doesnt have titlebar but still has frame (should really have? - in case you want custom frame) and is dragable (by custom area)
	WD_NONE, // no titlebar + no frame // completely frameless (on wayland is layer shell window), used for popups, currently called unmanaged
	
	//WD_CLIENT_FULL, // bar (=topmost, cmd, rotate, ... ~delivered by HUI)
	//WD_CLIENT_MINI, // in corner (semitransparent, shown on hover ~delivered by HUI) // buttons in specified corner of the window BUT no titlebar (hybrid server+client)
};


enum WindowState { // currently unused  // TODO: window state is part of window geometry, 
	
	WS_KEEPSIZE=0, WS_NORMAL=0,
	WS_MAXIMIZED=1,
	WS_FULLSCREEN=2,
	
    WS_DEFAULT=-1 // let the wm decide (usually last known for the app or current)
	//WS_UNSET=-1 // todo, should work the same as default
	
	// WS_TILING_x,  // not possible (at least with gtk3) - instead position+size+anchors should be used  
	// WS_MINIMIZED  // the window can be both maximized/keepsize and minimized (conify/deiconify) = its more position_z than state BUT window can be both minimized and set to topmost at the same time BUT what about layer shell (btw window with size 0x0 is in reality 1 pixel)
	
	// https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/API/windows/WindowState
	// this is not how it should work: https://learn.microsoft.com/en-us/dotnet/api/system.windows.windowstate?view=windowsdesktop-8.0
	// some ideas (about non standart wms): https://www.reddit.com/r/linux/comments/16vciex/lets_make_wayland_have_support_for_window/
	
};



};
#endif // HUI_WINDOW_TYPES_H