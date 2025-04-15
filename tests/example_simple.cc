#include <iostream>
#include "HUI.hh"
//#include <ciso646>
//#include "hui_datatypes.h" // deprecated

const char* html =  R"str(<!DOCTYPE html>
<html>
  <head>
  </head>
  <body>
    <button>click me</button>
  </body>
<html>
)str";

int main (int argc, char* argv[]){
	
	HUI::WebView window;
	//window.load_file(HUI::filepath("simple_example.html").c_str()); // old api
	window.load_str(html);

	window.html_element("button", "onclick", window.call_native([&window](std::vector<std::string>) -> void {
		window.call_js("document.querySelector('button').hidden = true;");
		//window.html_element("button", "hidden", true);  // equivalent way
		std::cout<<"button removed";
		}));
	
	HUI::WebView::handle_forever();
}