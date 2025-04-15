#include "HUI.h"


HUI_WebView window1 = NULL;
HUI_WebView window2 = NULL;

void callback (char** args, int num, void* data){
	printf("callback called\n");
}

void toggle_window (char** args, int num, void* data){  // create second window if its not cerated, otherwise destroy it
	if (window2 == NULL) {
		window2 = HUI_WebView_create ();
		if (data != NULL) {
			HUI_WebView_load_uri (window2, (const char*) data);
		}
		else {
			HUI_WebView_load_str (window2, "<!DOCTYPE html>\n<html><body><p> nothing to show </p></body></html>");
		}
		
	}
	else {
		HUI_WebView_destroy (window2);
		window2 = NULL;
	}
}

void quit (char** args, int num, void* data){  // close all windows and exit
	HUI_WebView_exit ();
}


int main (int argc, char* argv[]){
	
	window1 = HUI_WebView_create ();
	
	// add buttons
	HUI_WebView_html_element (window1, "body button#callback", "", "");
	HUI_WebView_html_element (window1, "button#callback", "innerText", "'run callback'");
	HUI_WebView_html_element (window1, "button#callback", "onclick", HUI_WebView_call_native(window1,&callback,NULL,"function(...args_array){return args_array}"));
	
	HUI_WebView_html_element (window1, "body button#window", "", "");
	HUI_WebView_html_element (window1, "button#window", "innerText", "'toggle window'");
	if (argc == 2) {
		HUI_WebView_html_element (window1, "button#window", "onclick", HUI_WebView_call_native(window1,&toggle_window,(void*)argv[1],"function(...args_array){return args_array}"));
	}
	else {
		HUI_WebView_html_element (window1, "button#window", "onclick", HUI_WebView_call_native(window1,&toggle_window,NULL,"function(...args_array){return args_array}"));
	}
	
	HUI_WebView_html_element (window1, "body button#quit", "", "");
	HUI_WebView_html_element (window1, "button#quit", "innerText", "'close and exit'");
	HUI_WebView_html_element (window1, "button#quit", "onclick", HUI_WebView_call_native(window1,&quit,NULL,"function(...args_array){return args_array}"));
	
	
	HUI_WebView_handle_forever ();  // enter endless message loop 
}