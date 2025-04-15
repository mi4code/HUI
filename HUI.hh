#ifndef _HUI_hh_
#define _HUI_hh_

#include "HUI.h"

#include <string>
#include <vector>
#include <functional>
#include <memory>


namespace HUI {


class 
  #if defined(_hui_backend_) && !defined(_MSC_VER) 
  __attribute__((visibility("hidden"))) // hides C++ functions from dynamic library
  #endif
  WebView {
	public:
	  WebView ();
	  ~WebView ();
	  
	  void load_file (std::string file); 
	  void load_uri (std::string uri); 
	  void load_str (std::string str); 
	  void hui_tweaks ();  // (transparency), hui.js, hui.css, theme
	  
	  std::string call_js (std::string code, bool return_data=false); //..., frame=*);
	  void call_js (std::string code, std::function<void(std::string)> return_callback);
	  std::string call_native (std::function<void(std::vector<std::string>)> handler, std::string process_args="function(...args_array){return args_array}");
	  std::string html_element (std::string query, std::string property="", std::string value=""); // TODO: return and value should have type for js values + make return async
	  
	  std::string backend_name ();
	  void* backend_object ();
	  std::string window_type ();
	  void* window_handle ();
	  
	  static void handle_forever ();
	  static void handle_once (); // can be used like `while(1){ww.handle_once(); usleep(35000);}`
	  static void/*std:thread*/ handle_threaded ();
	  static void exit ();

	// TODO: implicitly convert to c backend pointer if possible (shouldnt be member) 

	#if !defined(_hui_backend_)
	private:  // private when used as header (used when building backends - thats the only time you know whats really inside)
	#endif
      //friend class WindowControls;
	  struct pImpl;
	  std::unique_ptr<pImpl> impl; 
};


typedef HUI_WindowType WindowType;
typedef HUI_WindowLayer WindowLayer;
typedef HUI_WindowPixels WindowPixels;
typedef HUI_WindowMonitor WindowMonitor;
typedef HUI_WindowState WindowState;
typedef HUI_WindowGeometry WindowGeometry; // TODO: C++ (not possible in C) union aliases (x,y,w,h) and default values (-1)
typedef HUI_WindowInputMode WindowInputMode;
class 
  #if defined(_hui_backend_) && !defined(_MSC_VER) 
  __attribute__((visibility("hidden"))) // hides C++ functions from dynamic library
  #endif
  WindowControls {
    public:
	  WindowControls(void* backend, void* handle); // for gtk3 its pointer to GtkWidget* so GtkWidget**
	  ~WindowControls();
	  
	  void set_type (WindowType type);
	  WindowType get_type ();
	  
	  void set_layer (WindowLayer layer);
	  WindowLayer get_layer ();
	  
	  void set_geometry (WindowGeometry geometry); // TODO: layer shel move vs. cover = 0 vs -1
	  WindowGeometry get_geometry ();
	  
	  void set_opacity (uint8_t opacity);
	  uint8_t get_opacity ();
	  
	  void set_id (std::string id);
	  std::string get_id ();
	  
	  void set_title (std::string title);
	  std::string get_title ();
	  
	  void set_exclusive_zone (int16_t zone);
	  int16_t get_exclusive_zone ();
	  
	  void set_focused (bool activated); // TODO: decide name focused=activated=stacking_position
	  bool get_focused (); 
	  
	  void set_input_mode_keyboard (WindowInputMode mode); // TODO: now copies wayland layer-shell protocol, should be refactored, decide name (focus_policy)
	  WindowInputMode get_input_mode_keyboard ();
	  
	  // TODO: callback for each of these (there should be some easy way to set/unset them; consider one callback vs. multiple)

    private: 
	  struct pImpl;
	  std::unique_ptr<pImpl> impl; 
	  
};


} // HUI


#if !defined(_hui_backend_)  // C API -> C++ API (used when including the library)

namespace HUI {


struct WebView::pImpl {
	HUI_WebView pointer;
};

WebView::WebView () {
	impl = std::make_unique<pImpl>();
	impl->pointer = HUI_WebView_create();
}
WebView::~WebView () {
	HUI_WebView_destroy(impl->pointer);
}	

void WebView::load_file (std::string file){
	HUI_WebView_load_file(impl->pointer, file.c_str());
}
void WebView::load_uri (std::string uri){
	HUI_WebView_load_uri(impl->pointer, uri.c_str());
}
void WebView::load_str (std::string str){
	HUI_WebView_load_str(impl->pointer, str.c_str());
}
void WebView::hui_tweaks (){
	HUI_WebView_hui_tweaks(impl->pointer);
}

std::string WebView::call_js (std::string code, bool return_data){
	auto cstr = HUI_WebView_call_js (impl->pointer, code.c_str(), return_data);
	auto str = std::string(cstr);
	delete[] cstr;
	return str;
}
void WebView::call_js (std::string code, std::function<void(std::string)> return_callback){
	void(*callback_c)(const char*, void*) = [](const char* arg, void* data)->void{
		static_cast<std::function<void(std::string)>*>(data)->operator()(std::string(arg));
	};
	#if defined(_MSC_VER) or defined(HUI_PYTHON)
		// SEE NOTE IN call_native
		std::function<void(std::string)>* callback_ptr = new std::function<void(std::string)>;
		callback_ptr->operator=( return_callback ); 
		
		HUI_WebView_call_js_async (impl->pointer, code.c_str(), callback_c, callback_ptr);
	#else
		// for g++
		HUI_WebView_call_js_async (impl->pointer, code.c_str(), callback_c, &return_callback);
	#endif
}
std::string WebView::call_native (std::function<void(std::vector<std::string>)> handler, std::string process_args){
	void(*handler_c)(char**, int, void*) = [](char** args, int len, void* data)->void{
		std::vector<std::string> args_cpp (args, args + len);
		static_cast<std::function<void(std::vector<std::string>)>*>(data)->operator()(args_cpp);
	};
	#if defined(_MSC_VER) or defined(HUI_PYTHON)
		// for MSVC cl.exe
		/* fixes issue that caused apps to crash when callback was called form js,
		   which was likely caused by the garbage collector, that way that since handler std::function was created inside the argument list, its lifetime ended after return of this function (so it didnt exist when it was called) 
		   solved by following line that (creates memory leak) creates new object as a pointer and copies the function there (original fastfix solution was to change handler argument type to pointer which could be created in main of app code)
           (note: there was similar issue with C API const char* return values - not sure but i think it was different set of compilers) */
		// same situation happens with python bindings - when built to link to libHUI (my case was qt5+windows+mingw, not sure about other backends/platforms) and not include the backend (worked fine in the same case)
		std::function<void(std::vector<std::string>)>* handler_ptr = new std::function<void(std::vector<std::string>)>;
		handler_ptr->operator=( handler ); 
		
		auto cstr = HUI_WebView_call_native (impl->pointer, handler_c, handler_ptr, process_args.c_str());
	#else
		// for g++
		auto cstr = HUI_WebView_call_native (impl->pointer, handler_c, &handler, process_args.c_str());
	#endif
	auto str = std::string(cstr);
	delete[] cstr;
	return str;
}
std::string WebView::html_element (std::string query, std::string property, std::string value){
	auto cstr =HUI_WebView_html_element (impl->pointer, query.c_str(), property.c_str(), value.c_str());
	auto str = std::string(cstr);
	delete[] cstr;
	return str;
}

std::string WebView::backend_name (){
	auto cstr = HUI_WebView_backend_name (impl->pointer);
	auto str = std::string(cstr);
	delete[] cstr;
	return str;
}
void* WebView::backend_object (){
	return HUI_WebView_backend_object(impl->pointer);
	//return impl->pointer;  // use if you want to combine C++ and C (and dont care about the backend)
}
std::string WebView::window_type (){
	auto cstr = HUI_WebView_window_type (impl->pointer);
	auto str = std::string(cstr);
	delete[] cstr;
	return str;
}
void* WebView::window_handle (){
	return HUI_WebView_window_handle(impl->pointer);
}

void WebView::handle_forever (){
	HUI_WebView_handle_forever();
}
void WebView::handle_once (){
	HUI_WebView_handle_once();
}
void/*std:thread*/ WebView::handle_threaded (){
	/*return*/ HUI_WebView_handle_threaded();
}
void WebView::exit (){
	HUI_WebView_exit();
}


struct WindowControls::pImpl {
    HUI_WindowControls pointer;
};

WindowControls::WindowControls(void* backend, void* handle) { // doesnt work when use is of type WebView (may be type mismatch (expected to be backend type BUT its bundings type -- but still strange because it should have been link compatible)
	impl = std::make_unique<pImpl>();
    impl->pointer = HUI_WindowControls_create(backend, handle);
}
WindowControls::~WindowControls() {
    HUI_WindowControls_destroy(impl->pointer);
}

void WindowControls::set_type(WindowType type) {
    HUI_WindowControls_set_type(impl->pointer, type);
}
WindowType WindowControls::get_type() {
    return HUI_WindowControls_get_type(impl->pointer);
}

void WindowControls::set_layer(WindowLayer layer) {
    HUI_WindowControls_set_layer(impl->pointer, layer);
}
WindowLayer WindowControls::get_layer() {
    return HUI_WindowControls_get_layer(impl->pointer);
}

void WindowControls::set_geometry(WindowGeometry geometry) {
    HUI_WindowControls_set_geometry(impl->pointer, geometry);
}
WindowGeometry WindowControls::get_geometry() {
    return HUI_WindowControls_get_geometry(impl->pointer);
}

void WindowControls::set_opacity (uint8_t opacity) {
	HUI_WindowControls_set_opacity(impl->pointer, opacity);
}
uint8_t WindowControls::get_opacity () {
    return HUI_WindowControls_get_opacity(impl->pointer);
}

void WindowControls::set_title(std::string title) {
    HUI_WindowControls_set_title(impl->pointer, title.c_str());
}
std::string WindowControls::get_title() {
    auto cstr = HUI_WindowControls_get_title(impl->pointer);
    auto str = std::string(cstr);
    delete[] cstr;
    return str;
}

void WindowControls::set_id(std::string id) {
    HUI_WindowControls_set_id(impl->pointer, id.c_str());
}
std::string WindowControls::get_id() {
    auto cstr = HUI_WindowControls_get_id(impl->pointer);
    auto str = std::string(cstr);
    delete[] cstr;
    return str;
}

void WindowControls::set_exclusive_zone(int16_t zone) {
    HUI_WindowControls_set_exclusive_zone(impl->pointer, zone);
}
int16_t WindowControls::get_exclusive_zone() {
    return HUI_WindowControls_get_exclusive_zone(impl->pointer);
}

void WindowControls::set_focused(bool activated) {
    HUI_WindowControls_set_focused(impl->pointer, activated);
}
bool WindowControls::get_focused() {
    return HUI_WindowControls_get_focused(impl->pointer);
}

void WindowControls::set_input_mode_keyboard(WindowInputMode mode) {
    HUI_WindowControls_set_input_mode_keyboard(impl->pointer, mode);
}
WindowInputMode WindowControls::get_input_mode_keyboard() {
    return HUI_WindowControls_get_input_mode_keyboard(impl->pointer);
}


} // HUI

#endif // _hui_backend_


#if defined(_hui_backend_)  // C++ API -> C API (used when building the library)


#ifdef __cplusplus
extern "C" {
#endif


#include "HUI.h"
#include <cstring>


HUI_WebView HUI_WebView_create (){
	return new HUI::WebView();
}
void        HUI_WebView_destroy (HUI_WebView object){
	delete static_cast<HUI::WebView*>(object);
}

void        HUI_WebView_load_file (HUI_WebView object, const char* file){
	static_cast<HUI::WebView*>(object)->load_file(file);
}
void        HUI_WebView_load_uri (HUI_WebView object, const char* uri){
	static_cast<HUI::WebView*>(object)->load_uri(uri);
}
void        HUI_WebView_load_str (HUI_WebView object, const char* str){
	static_cast<HUI::WebView*>(object)->load_str(str);
}
void        HUI_WebView_hui_tweaks (HUI_WebView object){
	static_cast<HUI::WebView*>(object)->hui_tweaks();
}

const char* HUI_WebView_call_js (HUI_WebView object, const char* code, bool return_data){ 
	auto str = static_cast<HUI::WebView*>(object)->call_js(code, return_data);
	char* cstr = new char[str.size() + 1]; // from <https://stackoverflow.com/a/22330685/21213243>, its important to call delete
	strcpy(cstr, str.c_str());
	return cstr;
}
void        HUI_WebView_call_js_async (HUI_WebView object, const char* code, void(*return_callback)(const char*, void*), void* data){
	auto cpp_callback = [data,return_callback](std::string arg)->void{
		const char* c_arg = const_cast<const char*>(arg.c_str());
		return_callback(c_arg,data); //segfault (if handler captured by reference as &handler)
	};
	static_cast<HUI::WebView*>(object)->call_js(code, cpp_callback);
}
const char* HUI_WebView_call_native (HUI_WebView object, void(*handler)(char**,int,void*), void* data, const char* process_args){
	auto cpp_handler = [data,handler](std::vector<std::string> args)->void{
		// from <https://stackoverflow.com/a/26032303/21213243>
		std::vector<char*> c_args;
		c_args.reserve(args.size());
		for(size_t i = 0; i < args.size(); ++i) c_args.push_back(const_cast<char*>(args[i].c_str()));
		handler(&c_args[0], c_args.size(),data); //segfault (if handler captured by reference as &handler)
	};
	auto str = static_cast<HUI::WebView*>(object)->call_native(cpp_handler, process_args);
	char* cstr = new char[str.size() + 1]; // from <https://stackoverflow.com/a/22330685/21213243>, its importent to call delete
	strcpy(cstr, str.c_str());
	return cstr;
}
const char* HUI_WebView_html_element (HUI_WebView object, const char* query, const char* property, const char* value){
	auto str = static_cast<HUI::WebView*>(object)->html_element(query, property, value);
	char* cstr = new char[str.size() + 1]; // from <https://stackoverflow.com/a/22330685/21213243>, its importent to call delete
	strcpy(cstr, str.c_str());
	return cstr;
}

const char* HUI_WebView_backend_name (HUI_WebView object){ 
	auto str = static_cast<HUI::WebView*>(object)->backend_name();
	char* cstr = new char[str.size() + 1]; // from <https://stackoverflow.com/a/22330685/21213243>, its important to call delete
	strcpy(cstr, str.c_str());
	return cstr;
}
void*       HUI_WebView_backend_object (HUI_WebView object){
	return static_cast<HUI::WebView*>(object)->backend_object();
}
const char* HUI_WebView_window_type (HUI_WebView object){ 
	auto str = static_cast<HUI::WebView*>(object)->window_type();
	char* cstr = new char[str.size() + 1]; // from <https://stackoverflow.com/a/22330685/21213243>, its important to call delete
	strcpy(cstr, str.c_str());
	return cstr;
}
void*       HUI_WebView_window_handle (HUI_WebView object){
	return static_cast<HUI::WebView*>(object)->window_handle();
} 

void        HUI_WebView_handle_forever (){
	HUI::WebView::handle_forever();
}
void        HUI_WebView_handle_once (){
	HUI::WebView::handle_once();
}
void        HUI_WebView_handle_threaded (){
	HUI::WebView::handle_threaded();
}
void        HUI_WebView_exit (){
	HUI::WebView::exit();
}



HUI_WindowControls HUI_WindowControls_create (void* backend, void* handle){
	return new HUI::WindowControls(backend, handle);
	//return new HUI::WindowControls(static_cast<HUI::WebView*>(use));
}
void        HUI_WindowControls_destroy (HUI_WindowControls object){
	delete static_cast<HUI::WindowControls*>(object);
}

void        HUI_WindowControls_set_type (HUI_WindowControls object, HUI_WindowType type){
	static_cast<HUI::WindowControls*>(object)->set_type(type);
}
HUI_WindowType HUI_WindowControls_get_type (HUI_WindowControls object){
	return static_cast<HUI::WindowControls*>(object)->get_type();
}

void        HUI_WindowControls_set_layer (HUI_WindowControls object, HUI_WindowLayer layer){
	static_cast<HUI::WindowControls*>(object)->set_layer(layer);
}
HUI_WindowLayer HUI_WindowControls_get_layer (HUI_WindowControls object){
	return static_cast<HUI::WindowControls*>(object)->get_layer();
}

void        HUI_WindowControls_set_geometry (HUI_WindowControls object, HUI_WindowGeometry geometry){
	static_cast<HUI::WindowControls*>(object)->set_geometry(geometry);
}
HUI_WindowGeometry HUI_WindowControls_get_geometry (HUI_WindowControls object){
	return static_cast<HUI::WindowControls*>(object)->get_geometry();
}

void        HUI_WindowControls_set_opacity (HUI_WindowControls object, uint8_t opacity){
	static_cast<HUI::WindowControls*>(object)->set_opacity(opacity);
}
uint8_t     HUI_WindowControls_get_opacity (HUI_WindowControls object){
	return static_cast<HUI::WindowControls*>(object)->get_opacity();
}

void        HUI_WindowControls_set_title (HUI_WindowControls object, const char* title){
	static_cast<HUI::WindowControls*>(object)->set_title(title);
}
const char* HUI_WindowControls_get_title (HUI_WindowControls object){
	auto str = static_cast<HUI::WindowControls*>(object)->get_title();
	char* cstr = new char[str.size() + 1];
	strcpy(cstr, str.c_str());
	return cstr;
}

void        HUI_WindowControls_set_id (HUI_WindowControls object, const char* id){
	static_cast<HUI::WindowControls*>(object)->set_id(id);
}
const char* HUI_WindowControls_get_id (HUI_WindowControls object){
	auto str = static_cast<HUI::WindowControls*>(object)->get_id();
	char* cstr = new char[str.size() + 1];
	strcpy(cstr, str.c_str());
	return cstr;
}

void        HUI_WindowControls_set_exclusive_zone (HUI_WindowControls object, uint16_t zone){
	static_cast<HUI::WindowControls*>(object)->set_exclusive_zone(zone);
}
uint16_t    HUI_WindowControls_get_exclusive_zone (HUI_WindowControls object){
	return static_cast<HUI::WindowControls*>(object)->get_exclusive_zone();
}

void        HUI_WindowControls_set_focused (HUI_WindowControls object, bool activated){
	static_cast<HUI::WindowControls*>(object)->set_focused(activated);
}
bool        HUI_WindowControls_get_focused (HUI_WindowControls object){
	return static_cast<HUI::WindowControls*>(object)->get_focused();
}

void        HUI_WindowControls_set_input_mode_keyboard (HUI_WindowControls object, HUI_WindowInputMode mode){
	static_cast<HUI::WindowControls*>(object)->set_input_mode_keyboard(mode);
}
HUI_WindowInputMode     HUI_WindowControls_get_input_mode_keyboard (HUI_WindowControls object){
	return static_cast<HUI::WindowControls*>(object)->get_input_mode_keyboard();
}


#ifdef __cplusplus
}
#endif


#endif


#if defined(_hui_webview_use_common_code)


namespace HUI {
	
std::string WebView::html_element (std::string query, std::string property, std::string value){
	if (property == "" and value == ""){ // create element
		// user should do outerHTML manually
		// parent must already exist, the rest should be only tag#id.class.another[attr=val][another="val"][prop='val']
		call_js( std::string()+"HUI.html_element_create(`"+query+"`)", false );
		return "";
	}
	else if (value == "") { // get value or call 
		// query must be unique to get value
		return call_js( std::string()+"(function(){ let q = document.querySelectorAll('"+query+"'); if (q.length == 1) {return q[0]."+property+";} else {q.forEach(function(e){e."+property+";}); return '<hui_error: query not unique>';} })();", true );
	}
	else { // set value
		call_js( std::string()+"(function(){  let q = document.querySelectorAll('"+query+"'); if (q.length == 1) {q[0]."+property+" = "+value+";} else {q.forEach(function(e){e."+property+" = "+value+";});}  })()", false );
		return "";
	}
	
	/*

// no return when value has value
// if you want element from html string: use outerHTML + run all scripts
// query may contain ':nth-of-type(...)' to distinguish between similar elements ('querySelector' is always faster than 'querySelectorAll')

TESTS PERFORMED TO FIND BEST APPROACH:

tested with: <https://jsbench.me>

---

<html>
<body>
<div style="color: red;"></div>
<button>
<div></div>
<div></div>
<div></div>
<p></p>
<p></p>
<div></div>
</body>
</html>
 [TEST SETUP]

---

document.querySelectorAll("body div:nth-of-type(3)").forEach(function(e){e.innerText = "e";})
 [261 tis. ops/s ± 6.61% ~ 15.62 % slower]

(function(e){e.innerText = "e";})(document.querySelectorAll("body div")[2])
 [299 tis. ops/s ± 1.55% ~ 3.39 % slower]
 
document.querySelectorAll("body div")[2].innerText = "e";
 [303 tis. ops/s ± 2.73% ~ Fastest]
 
let q = document.querySelectorAll("body div:nth-of-type(3)");
if (q.length == 1) {q[0].innerText = "e";}
else {q.forEach(function(e){e.innerText = "e";})}
 [309 tis. ops/s ± 1.07% ~ Fastest]

---

document.querySelectorAll("body div").forEach(function(e){e.innerText = "e";})
 [308 tis. ops/s ± 2.95% ~ 18.12 % slower]
 
let q = document.querySelectorAll("body div");
if (q.length == 1) {q[0].innerText = "e";}
else {q.forEach(function(e){e.innerText = "e";})}
 [377 tis. ops/s ± 2.9% ~ Fastest]
 
 
 
 
*/

}

void WebView::call_js (std::string code, std::function<void(std::string)> return_callback) { // TODO: unify js api (this is just placeholder for backends using synchronous api)
	return_callback( call_js(code, true) );
	//return_callback( (this->*static_cast<std::string(WebView::*)(std::string,bool)>(&WebView::call_js))(code, true) );
}

/*void call_js (std::string code, bool return_data=False) {  // TODO: unify js api
	if (return_data) { // TODO: wouldnt 3 members be better (sometimes there are 2 different ways to call js)
		// TODO: ...while with timeout and handle_once (dont forget that cef is weird)
	}
	else {
		call_js(code, [](std::string){});
		return "";
	}
	return_callback( call_js(code, true) );
}*/

} // HUI

#endif // _hui_webview_use_common_code


#endif // _HUI_hh_