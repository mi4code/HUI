#ifndef _hui_backend_
#define _hui_backend_

#define _hui_webview_use_common_code
#include "HUI.hh"

#include "./hui_datatypes.h"
#include <fstream>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkwayland.h>
#include <webkit2/webkit2.h>
#include <gtk-layer-shell.h>
#include <iostream>


namespace HUI {

std::vector< std::function<void(std::vector<std::string>)> > sent2cpp_handlers;

bool gtk_inited = false;


struct WebView::pImpl {
	GtkWidget* window = NULL;
	GtkWidget* webview = NULL;
};

WebView::WebView () {
	
	impl = std::make_unique<pImpl>();
	
	if (!gtk_inited) {gtk_init(NULL, NULL);  gtk_inited = true;}  // init so no extra global init is needed
	
	// webview
	impl->webview = GTK_WIDGET(webkit_web_view_new_with_context(webkit_web_context_new_ephemeral())); // private mode (should be always enabled)
	//webkit_settings_set_enable_private_browsing (webkit_web_view_get_settings(webview), true); // deprecated private mode
	//if (!webkit_web_context_get_sandbox_enabled ( webkit_web_view_get_context (webview) )) {std::cout<<"not sandboxed";} // not sandboxed by default
	
	// transparency
	gtk_widget_set_visual(GTK_WIDGET(impl->webview), gdk_screen_get_rgba_visual(gdk_screen_get_default()));
	GdkRGBA transparent = {0,0,0,0};
	webkit_web_view_set_background_color(WEBKIT_WEB_VIEW(impl->webview), &transparent);
	
	// hui.js + hui.css + HUI_theme.css
	hui_tweaks();
	
	// just to unify all backends (not needed here)
	//webkit_web_view_load_uri(WEBKIT_WEB_VIEW(impl->webview), "about:blank");

	// debug
	#if defined(HUI_DEBUG_ENABLED)
		HUI_DEBUG_PRINT("enabling devtools ");
		webkit_settings_set_enable_developer_extras(webkit_web_view_get_settings(WEBKIT_WEB_VIEW(impl->webview)), true);
		webkit_web_inspector_show(webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(impl->webview))); // takes extra RAM (about the same as the app itself)
	#endif
	
	// js<-->cpp
	WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager (WEBKIT_WEB_VIEW(impl->webview));
	g_signal_connect (manager, "script-message-received::sent2cpp", G_CALLBACK(+[](WebKitUserContentManager* manager, WebKitJavascriptResult* message, gpointer user_data) -> void {

		JSGlobalContextRef context = webkit_javascript_result_get_global_context(message);
		JSCValue* result = webkit_javascript_result_get_js_value(message);
		std::string msgstr = std::string(jsc_value_to_string(result));
		
		HUI_DEBUG_PRINT("call from js to cpp '"<<msgstr<<"' ");
		
		const int hi = stoi(msgstr.substr(0,msgstr.find(",")));
		msgstr = msgstr.substr(msgstr.find(",")+1+1, msgstr.size()-1 -(msgstr.find(",")+1+1) );
		
		std::vector<HUI::Str> msgdata_ = HUI::Str(msgstr).split("\",\"");
		std::vector<std::string> msgdata;
		for (auto it : msgdata_) msgdata.push_back ( it.replace("\\\"","\"").cpp_str() );

		sent2cpp_handlers.at(hi)(msgdata);
		
	}), NULL);

	webkit_user_content_manager_register_script_message_handler (manager, "sent2cpp");
	
	// window
	impl->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	// TODO: handle close callback - maybe from js side <https://stackoverflow.com/questions/821011/prevent-a-webpage-from-navigating-away-using-javascript>
	//g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL); 
	//g_signal_connect(window, "destroy", G_CALLBACK(+[](){ gtk_main_quit(); }), NULL);
	
	// transparency
	gtk_widget_set_app_paintable(GTK_WIDGET(impl->window), TRUE);
	gtk_widget_set_visual(GTK_WIDGET(impl->window), gdk_screen_get_rgba_visual(gdk_screen_get_default()));
	
	// add
	gtk_container_add(GTK_CONTAINER(impl->window), GTK_WIDGET(impl->webview));
	
	// show - this makes the window show up - but after that, some controls may not work (in that case the window is destroyed and the webview is moved to a new window; there may be alternative aproach by calling this once we get to the massage loop handling - <https://docs.gtk.org/glib/func.idle_add_once.html>)
	gtk_widget_show_all(GTK_WIDGET(impl->window)); 
	
	// TODO: kill 'WebKitNetworkProcess' to save memory (if the app works offline - it should)
	
}

WebView::~WebView () {
	gdk_window_destroy (gtk_widget_get_window(impl->window));
}


void WebView::load_file (std::string file){
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(impl->webview), std::string("file://"+file).c_str() );
	hui_tweaks();
}

void WebView::load_uri (std::string uri){
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(impl->webview), uri.c_str() );
	hui_tweaks();
}

void WebView::load_str (std::string str){
	webkit_web_view_load_html (WEBKIT_WEB_VIEW(impl->webview), str.c_str() , "none");
	hui_tweaks();
}

void WebView::hui_tweaks (){ 
	
	WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager (WEBKIT_WEB_VIEW(impl->webview));
	
	// if this looks like this loads the script for all future pages, its not like that (it has to be run every time) 
	webkit_user_content_manager_add_script(manager, webkit_user_script_new(
		#include "hui.js"
		,WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, NULL,NULL) ); 
		
	call_js(std::string(
		"style = document.createElement('style'); style.textContent = \"")+
		HUI::Str(
		#include "hui.css" // JSERROR{unexpected EOF} --fixed
		).replace("\n","\\n").replace("\"","\\\"").cpp_str()
		+std::string("\"; document.head.append(style);"
		).c_str(),false);


	std::string css;
	std::ifstream filestream (HUI::filepath(
	#if defined(_WIN32)
		"%userprofile%/HUI_theme.css"
	#else
		"$HOME/.config/HUI_theme.css"
	#endif
	).c_str());
	if (filestream.good()) {
		// same as following oneliner
		//std::stringstream stringstream;
		//stringstream << filestream.rdbuf();
		//std::string str = stringstream.str();
		//css = str;
		css = std::string((std::istreambuf_iterator<char>(filestream)), std::istreambuf_iterator<char>()); // TODO: the fucking file must have unix line ends (not windows ones) - otherwise it breaks the js parsing resulting in "Unexpected EOF"
	}
	else {
		css = 
		#include "HUI_theme.css"
		;
	}
	call_js(std::string(
		"style = document.createElement('style'); style.textContent = \"")+
		std::string(HUI::Str(std::string(css.c_str())).replace("\n","\\n").replace("\"","\\\"").c_str())
		+std::string("\"; document.head.append(style);"
		),false);
	
	/*WebKitUserContentManager *manager = webkit_web_view_get_user_content_manager (WEBKIT_WEB_VIEW(webview));
	
	webkit_user_content_manager_add_script(manager, webkit_user_script_new(
		#include "hui.js"
		,WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, NULL,NULL) ); 

	webkit_user_content_manager_add_style_sheet(manager, webkit_user_style_sheet_new ( // doesnt work, maybe?
		 #include "hui.css"
		,WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES, WEBKIT_USER_STYLE_LEVEL_AUTHOR, NULL,NULL) );*/
}


std::string WebView::call_js (std::string code, bool return_data){
	
	HUI_DEBUG_PRINT("call js from cpp '"<<code<<"' ");
	
	if (return_data) {
		struct upoint {HUI::Str str = ""; bool ok = false;} stat;
		
		webkit_web_view_evaluate_javascript (WEBKIT_WEB_VIEW(impl->webview), code.c_str(), -1, NULL, NULL, NULL, +[](GObject *object, GAsyncResult *result, gpointer user_data) -> void {
			JSCValue *value;
			GError *error = NULL;
			value = webkit_web_view_evaluate_javascript_finish (WEBKIT_WEB_VIEW (object), result, &error);
			((upoint*)user_data)->str = jsc_value_to_string (value);
			((upoint*)user_data)->ok = true;
		}, &stat );
		
		while (!stat.ok) handle_once();

		return stat.str.c_str();;
	}
	
	else {
		webkit_web_view_evaluate_javascript (WEBKIT_WEB_VIEW(impl->webview),code.c_str(),-1,NULL,NULL,NULL,NULL,NULL); //doesnt work at the begining of execution
		webkit_user_content_manager_add_script( webkit_web_view_get_user_content_manager (WEBKIT_WEB_VIEW(impl->webview)), webkit_user_script_new(code.c_str(),WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,NULL,NULL) );
		return "";
	}
	
}

std::string WebView::call_native (std::function<void(std::vector<std::string>)> handler, std::string process_args){
	sent2cpp_handlers.push_back(handler);
	auto dd = std::string()+"function(..._args_array){ window.webkit.messageHandlers['sent2cpp'].postMessage( '"+std::to_string(sent2cpp_handlers.size()-1)+"'+',\"'+("+process_args+")(_args_array).join('\",\"')+'\"' ); }";
	HUI_DEBUG_PRINT("created js function "<<dd);
	return dd;
}


std::string WebView::backend_name (){
	return "webkit-gtk3";
}

void* WebView::backend_object (){
	HUI_DEBUG_PRINT ("webview pointer... "<<impl->window);
	//return impl->window;
	return static_cast<void*>(&(impl->window)); // pointer to GtkWidget* == GtkWidget**
}

std::string WebView::window_type (){
	GdkDisplay *display = gtk_widget_get_display(impl->window);
	if (GDK_IS_X11_DISPLAY(display)) return "x11";
    else if (GDK_IS_WAYLAND_DISPLAY(display)) return "wayland";
	else return "unknown";
}

void* WebView::window_handle (){
    GdkWindow* gdk_window = gtk_widget_get_window(impl->window);
    if (!gdk_window) return nullptr;

    GdkDisplay* display = gtk_widget_get_display(impl->window);

    if (GDK_IS_X11_DISPLAY(display)) {
        Window x11_handle = gdk_x11_window_get_xid(gdk_window);
        return reinterpret_cast<void*>(x11_handle);
    }
	
    else if (GDK_IS_WAYLAND_DISPLAY(display)) {
        struct wl_surface* wayland_handle = gdk_wayland_window_get_wl_surface(gdk_window);
        return static_cast<void*>(wayland_handle);
    }
	
    return nullptr;
}


void WebView::handle_forever (){
	gtk_main();
}

void WebView::handle_once (){
	while (gtk_events_pending()) {gtk_main_iteration();}
}

void/*std:thread*/ WebView::handle_threaded (){
	/*return*/ // TODO
}

void WebView::exit (){
	gtk_main_quit();
	sent2cpp_handlers.clear(); // solves problem with python bindings when it was impossible to exit due to GIL error in gillstate_tss_set (not sure if still present?)
}


#include "./hui_windowcontrols__gtk3.cc"


} // HUI
#endif // _hui_backend_