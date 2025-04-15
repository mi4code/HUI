
#define HUI_GTK3_DEBUG
#define HUI_QT5_DEBUG


//#define HUI_BACKEND_QT5
//#define HUI_BACKEND_WINDOWS
//#include <windows.h>

//#include "../hui.h" 

#include "../hui_new.cc" // order-sensitive - has to come before header
#include "../HUI.hh"  

// TODO: complete + make this js api test + add define for experiments (same with ui buttons/inputs)

int main (int argc, char* argv[]){
	

	auto window = HUI::App("test controls - HUI",NULL,"hui").new_window();
	window.use(&window);
	//auto window = HUI::Window();

	//gtk_window_set_decorated(GTK_WINDOW(window.backend_window()), false);
	//gtk_window_set_titlebar(GTK_WINDOW(window.backend_window()), GTK_WIDGET(window.backend_widget()));
	//auto ph = gtk_label_new(NULL);

	
	window.content(HUI::filepath("test_controls.html").c_str());
	window.set_id("firefox");
	window.set_input_mode_keyboard(HUI::WIM_NEVER);
	window.set_type(HUI::WT_TOPLEVEL_CSD);
	
	//window.content(HUI::filepath("/T:/Workspace/li-os/subprojects/li-hui/tests/test_controls.html"));
	
	window.js_set_value("hui_window.set_unmanaged", window.to_js([&window](std::vector<std::string> args){ // TODO: fix
		//window.set_geometry({.screen = 1, .top = 0, .left = 0, .right = 0, .bottom = -1, .w = -1, .h = 100});
		//window.set_geometry(-1,-1,-1,-1,-1,-1,0,WS_FULLSCREEN|WS_MINIMIZED);
		//window.set_geometry(-1,-1,-1,-1,-1,-1,-1,WS_MAXIMIZED|WS_N_MINIMIZED);
		//auto g = window.get_geometry();
		//window.set_unmanaged();
		window.show();
		std::cout<<window.get_id();
		window.set_input_mode_keyboard(HUI::WIM_AUTO_WINDOW);
		//window.set_geometry(g);
		}));
	//window.js_set_value("body", "onmousedown", window.to_js([&window](std::vector<std::string> args){ window.start_resize_drag(); }));
	window.js_set_value("hui_window.set_title", window.to_js([&window](std::vector<std::string> args){ window.set_title(args[0].c_str()); }));
	//window.js_set_value("hui_window.set_geometry");
	window.js_set_value("#geometry","onchange", window.to_js([&window](std::vector<std::string>){
		/*window.set_geometry({
			.x = -1,
			.y = -1,
			.w = int(HUI::Str(window.js_get_value("#geometry #w", "value"))),
			.h = int(HUI::Str(window.js_get_value("#geometry #h", "value")))
		});*/
		window.set_geometry(int(HUI::Str(window.js_get_value("#geometry #w", "value"))), int(HUI::Str(window.js_get_value("#geometry #h", "value"))),10,10);
	}));
	window.js_set_value("hui_window.set_layer", window.to_js([&window](std::vector<std::string> args){ 
		window.set_layer([&args](){ 
			switch(std::stoi(args[0].c_str())){
				case 0: return HUI::WL_BACKGROUND;
				case 1: return HUI::WL_BOTTOM;
				case 2: return HUI::WL_TOP;
				case 3: return HUI::WL_OVERLAY;
				default: return HUI::WL_TOP;
			}
			}() ); /*std::cout<<window.get_type();*/  std::cout<<int8_t(0b11111111); //window.set_geometry(-1,-1,-1,-1,-1,-1,-1,WS_N_FULLSCREEN); window.show();
		}));
	
	window.show();
	
	//HUI::handle_forever();
	while(1){HUI::handle_once();std::cout<<"q\n";}
}