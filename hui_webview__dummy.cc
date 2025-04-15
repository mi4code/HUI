#ifndef _hui_backend_
#define _hui_backend_


#define _hui_webview_use_common_code
#include "HUI.hh"

#include <iostream>

namespace HUI {

// std::vector< std::function<void(std::vector<std::string>)> > sent2cpp_handlers;

struct WebView::pImpl {};

WebView::WebView () {
	std::cout<<"WebView::WebView"<<"\n";
}
WebView::~WebView () {
	std::cout<<"WebView::~WebView"<<"\n";
}

void WebView::load_file (std::string file){
	std::cout<<"WebView::load_file"<<"\n\t"<<file<<"\n";
}
void WebView::load_uri (std::string uri){
	std::cout<<"WebView::load_uri"<<"\n\t"<<uri<<"\n";
}
void WebView::load_str (std::string str){
	std::cout<<"WebView::load_str"<<"\n\t"<<str<<"\n";
}
void WebView::hui_tweaks (){
	std::cout<<"WebView::hui_tweaks"<<"\n";
}

std::string WebView::call_js (std::string code, bool return_data){
	std::cout<<"WebView::call_js"<<"\n\t"<<code<<"\n\t"<<return_data<<"\n";
	return "";
}
std::string WebView::call_native (std::function<void(std::vector<std::string>)> handler, std::string process_args){
	std::cout<<"WebView::call_native"<<"\n\t"<<"-"<<"\n\t"<<process_args<<"\n";
	handler({"first","second","third"});
	// sent2cpp_handlers.push_back(handler);
	return "(function(){})";
}

std::string WebView::backend_name (){
	std::cout<<"WebView::backend_name"<<"\n";
	return "none-dummy";
}
void* WebView::backend_object (){
	std::cout<<"WebView::backend_object"<<"\n";
	return NULL;
}
std::string WebView::window_type (){
	std::cout<<"WebView::window_type"<<"\n";
	return "unknown";
}
void* WebView::window_handle (){
	std::cout<<"WebView::window_handle"<<"\n";
	return NULL;
}

void WebView::handle_forever (){
	std::cout<<"WebView::handle_forever"<<"\n";
	// if (sent2cpp_handlers.size()) sent2cpp_handlers.at(0)({"first","second","third"});
	while(1){std::cout<<".";}
}
void WebView::handle_once (){
	std::cout<<"WebView::handle_once"<<"\n";
}
void/*std:thread*/ WebView::handle_threaded (){
	std::cout<<"WebView::handle_threaded"<<"\n";
	//[](){while(1){std::cout<<".";}}
}
void WebView::exit (){
	std::cout<<"WebView::exit"<<"\n";
}


#include "./hui_windowcontrols__dummy.cc"


} // HUI

#endif // _hui_backend_