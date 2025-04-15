
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
		
	// TODO: WindowControls

} 
