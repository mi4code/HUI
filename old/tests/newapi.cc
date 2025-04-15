
#define HUI_GTK3_DEBUG
#define HUI_QT5_DEBUG

#include <unistd.h>
//#include "../hui_window_qt5.h"
//#include "../hui_window_gtk3.h"
#include "../hui.h"

//#include <thread>



int main (int argc, char* argv[]){
	
	// std::thread th(new_fc); // can be ran in thread (but all the variables should be created in that thread)
	
	auto ww = HUI::Window();
	
	ww.content("/home/mex/experiments.html");
	//ww.content("/T:/Workspace/li-os/subprojects/li-hui/tests/experiments.html");
	//ww.content2("/home/mex/experiments.html",true);
	//ww.content2("<html><body><script>var abc = 23;</script><p onload=\"this.innerHTML = this.innerHTML+aaa;\">Hello World!</p></body></html>");
	
	//ww.set_unmanaged();
	//ww.set_geometry({.x = 50, .y = 25, .w = 400, .h = 300});
	ww.set_geometry({.screen = 1, .top = 0, .left = 0, .right = 0, .bottom = -1, .w = -1, .h = 100});
	

	//auto www = new HUI::Window(); //prepared (then use ->)
	auto www = HUI::Window(); //prepared
	
	//www.content("/home/mex/experiments.html");
	www.content("/T:/Workspace/li-os/subprojects/li-hui/tests/experiments.html");

//ww.js_set_value("body", "onpointerdown", [&ww](std::vector<std::string> a)->void{gtk_window_begin_resize_drag( GTK_WINDOW(ww.backend_window()), GDK_WINDOW_EDGE_SOUTH_EAST/*todo auto*/, NULL, NULL, NULL, NULL); });

	www.js_set_value("body", "onclick", [&ww](std::vector<std::string> a)->void{
		ww.set_geometry({.fullscreen = true});
		ww.get_geometry(HUI::WindowGeometry::Maximized());
		//ww.content2("<html><body><script>var abc = 23;</script><p onload=\"this.innerHTML = this.innerHTML+'aaa';\">Hello World!</p></body></html>");
		//std::cout<<"cls"; ww.set_layer(HUI::WL_TOP);/*gdk_window_destroy(GDK_WINDOW(www.backend_object()));*/ /*ww.close();*/ 
		
		
		});

	ww.js_set_value("func",(std::function<void(std::vector<std::string>)>) [&ww](std::vector<std::string> a)->void{
		std::cout<<"js api test\n";
		
		//handle_once(); // works here
		
		std::cout<<"\n  the argument >>"<<a[0]<<"<<  \n";
		std::cout<<"\n  the return >>"<<ww.call_js("document.body.innerHTML",true).cpp_str()<<"<<  \n";
		
	  });
 
	ww.js_set_value(".gui_panel_item", "oncontextmenu", [&www, &ww](std::vector<std::string> a)->void{
		std::cout<<"right click menu window test\n";
		ww.set_geometry({.fullscreen = false});
		//ww.set_geometry({.x = 50, .y = 25, .w = 300, .h = 400});

		www.set_unmanaged(); // should be called before HUI::Window::show()
		www.set_geometry({.x = 100, .y = 75, .w = 80, .h = 100});
		
		www.js_set_value("body", "onclick", (std::function<void(std::vector<std::string>)>)[&www,&ww](std::vector<std::string> a)->void{ww.set_geometry({.minimized = false}); std::cout<<"cls__"; /*gdk_window_destroy(GDK_WINDOW(www.backend_object()));*/ }); // DOESNT WORK FOR SOME REASON !!!
		//www.set_input_mode_mouse(HUI::WIM_NEVER);
		//www.set_input_mode_mouse(HUI::WIM_ALWAYS);
		www.show();
		//ww.set_layer(HUI::WL_TOPLEAST);
		

	});

    ww.js_set_value_("button_click2", "function(){console.log('button 2');}");
    ww.call_js("var button_click = function(){console.log('button 1');};");
	
	// ((QWidget*) ww.backend_window())->show();  // use the backend object access this way
	// gdk_window_destroy(GDK_WINDOW(www.backend_object()));

	ww.show();
	
	/*ww.on_closing = [&www](){
		www.show();
		
	};*/
	
	HUI::handle_forever();
  
	return 0;
}