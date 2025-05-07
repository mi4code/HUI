
// pybind11 has to be included before Qt <https://github.com/pybind/pybind11/issues/2305>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

//#include "hui_webview__webkit_qt5.cc"
#define HUI_PYTHON  // fixes issue when callbacks being called caused exceptions/crashes when linking to libHUI (see HUI.hh)
#include "HUI.hh"


namespace py = pybind11;

using namespace HUI;

//std::function<void(std::vector<std::string>)> handlerca = [](std::vector<std::string> args)->void{ std::cout<<"\n"<<args[0]<<"\n";};


PYBIND11_MODULE(HUI, m) {
	
	py::class_<WebView>(m, "WebView")
		.def(py::init<>())
		
		.def("load_file", &WebView::load_file, py::arg("file"))
		.def("load_uri", &WebView::load_uri, py::arg("uri"))
		.def("load_str", &WebView::load_str, py::arg("str"))
		.def("hui_tweaks", &WebView::hui_tweaks)
		
		.def("call_js", static_cast<std::string(WebView::*)(std::string,bool)>(&WebView::call_js), py::arg("code"), py::arg("return_data") = false)
		.def("call_js", static_cast<void(WebView::*)(std::string,std::function<void(std::string)>)>(&WebView::call_js), py::arg("code"), py::arg("callback"))
		.def("call_native", &WebView::call_native, py::arg("handler"),  py::arg("process_args") = "function(...args_array){return args_array}")
		.def("html_element", &WebView::html_element, py::arg("query"), py::arg("property") = "", py::arg("value") = "")
		
		.def("backend_name", &WebView::backend_name)
		.def("backend_object", &WebView::backend_object) // TODO: return class instead of raw pointer (or add option to perform cast)
		.def("window_type", &WebView::window_type)
		.def("window_handle", &WebView::window_handle)
		
		.def("handle_forever", &WebView::handle_forever)
		.def("handle_once", &WebView::handle_once)
		.def("handle_threaded", &WebView::handle_threaded) // unsupported
		.def("exit", &WebView::exit)
		
		;

	
	py::class_<WindowControls>(m, "WindowControls")
		.def(py::init<void*, void*>(), py::arg("backend"), py::arg("handle"))
		
		.def("set_type", &WindowControls::set_type, py::arg("type"))
		.def("get_type", &WindowControls::get_type)
		
		.def("set_layer", &WindowControls::set_layer, py::arg("layer"))
		.def("get_layer", &WindowControls::get_layer)
		
		.def("set_geometry", &WindowControls::set_geometry, py::arg("geometry"))
		.def("get_geometry", &WindowControls::get_geometry)
		
		.def("set_opacity", &WindowControls::set_opacity, py::arg("opacity"))
		.def("get_opacity", &WindowControls::get_opacity)
		
		.def("set_id", &WindowControls::set_id, py::arg("id"))
		.def("get_id", &WindowControls::get_id)
		
		.def("set_title", &WindowControls::set_title, py::arg("title"))
		.def("get_title", &WindowControls::get_title)
		
		.def("set_exclusive_zone", &WindowControls::set_exclusive_zone, py::arg("zone"))
		.def("get_exclusive_zone", &WindowControls::get_exclusive_zone)
		
		.def("set_focused", &WindowControls::set_focused, py::arg("activated"))
		.def("get_focused", &WindowControls::get_focused)
		
		.def("set_input_mode_keyboard", &WindowControls::set_input_mode_keyboard, py::arg("mode"))
		.def("get_input_mode_keyboard", &WindowControls::get_input_mode_keyboard)
		
		;


	m.attr("WT_TOPLEVEL_SSD") = WT_TOPLEVEL_SSD;
	m.attr("WT_TOPLEVEL_CSD") = WT_TOPLEVEL_CSD;
	m.attr("WT_DESKTOP_COMPONENT") = WT_DESKTOP_COMPONENT;
	m.attr("WT_POPOVER") = WT_POPOVER;

	m.attr("WL_BACKGROUND") = WL_BACKGROUND;
	m.attr("WL_BOTTOM") = WL_BOTTOM;
	m.attr("WL_TOP") = WL_TOP;
	m.attr("WL_OVERLAY") = WL_OVERLAY;
	m.attr("WL_TOPLEAST") = WL_TOPLEAST;
	m.attr("WL_TOPMOST") = WL_TOPMOST;

	m.attr("WS_HIDDEN") = WS_HIDDEN;
	m.attr("WS_NOT_HIDDEN") = WS_NOT_HIDDEN;
	m.attr("WS_MINIMIZED") = WS_MINIMIZED;
	m.attr("WS_NOT_MINIMIZED") = WS_NOT_MINIMIZED;
	m.attr("WS_FULLSCREEN") = WS_FULLSCREEN;
	m.attr("WS_NOT_FULLSCREEN") = WS_NOT_FULLSCREEN;
	m.attr("WS_MAXIMIZED") = WS_MAXIMIZED;
	m.attr("WS_NOT_MAXIMIZED") = WS_NOT_MAXIMIZED;

	m.attr("WIM_ALWAYS") = WIM_ALWAYS;
	m.attr("WIM_AUTO_WINDOW") = WIM_AUTO_WINDOW;
	m.attr("WIM_NEVER") = WIM_NEVER;
	
} 
