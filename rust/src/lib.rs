use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void};

/*lazy_static::lazy_static! { // global storage for the handler
    static ref GLOBAL_HANDLER: Mutex<Option<Box<dyn Fn(Vec<String>) + Send + Sync>>> = Mutex::new(None);
}*/


pub struct WebView {
    pointer: *mut c_void,
}

impl WebView {
    
    pub fn new() -> Self {
        let pointer = unsafe { HUI_WebView_create() };
        Self { pointer }
    }


    pub fn load_file(&self, file: &str) {
        let file_cstr = CString::new(file).expect("CString::new failed");
        unsafe { HUI_WebView_load_file(self.pointer, file_cstr.as_ptr()) }
    }

    pub fn load_uri(&self, uri: &str) {
        let uri_cstr = CString::new(uri).expect("CString::new failed");
        unsafe { HUI_WebView_load_uri(self.pointer, uri_cstr.as_ptr()) }
    }

    pub fn load_str(&self, content: &str) {
        let content_cstr = CString::new(content).expect("CString::new failed");
        unsafe { HUI_WebView_load_str(self.pointer, content_cstr.as_ptr()) }
    }

    pub fn hui_tweaks(&self) {
        unsafe { HUI_WebView_hui_tweaks(self.pointer) }
    }


    pub fn call_js(&self, code: &str, return_data: Option<bool>) -> String {
        let code_cstr = CString::new(code).expect("CString::new failed");
        let result = unsafe { HUI_WebView_call_js(self.pointer, code_cstr.as_ptr(), return_data.unwrap_or(false)) };
        unsafe { CStr::from_ptr(result).to_string_lossy().into_owned() }
    }

	pub fn call_js_async<F>(&self, code: &str, callback: F) where F: Fn(&str) + Send + Sync + 'static + Clone, {
		let code_cstr = CString::new(code).expect("CString::new failed");

		let closure: Box<dyn Fn(&str)> = Box::new(callback.clone());  // box the closure
		let raw_fn: *mut dyn Fn(&str) = Box::into_raw(closure);  // convert the Box<dyn Fn()> to a raw pointer
		let thin_box: Box<*mut dyn Fn(&str)> = Box::new(raw_fn);  // box the fat pointer itself - this makes a thin pointer we can cast safely
		let callback_ptr: *mut c_void = Box::into_raw(thin_box) as *mut c_void;  // convert the Box<*mut dyn Fn()> to *mut c_void

		// Call the C function
		unsafe {
			HUI_WebView_call_js_async(
				self.pointer,
				code_cstr.as_ptr(),
				Some(Self::async_callback),
				callback_ptr,
			);
		}
	}
	unsafe extern "C" fn async_callback(result: *const c_char, data: *mut c_void) {

		// convert the result to a Rust string
		let result_str = if result.is_null() {
			String::from("")
		} else {
			CStr::from_ptr(result).to_string_lossy().into_owned()
		};

		let recovered_thin_box: Box<*mut dyn Fn(&str)> = unsafe { Box::from_raw(data as *mut *mut dyn Fn(&str)) };  // cast *mut c_void back to Box<*mut dyn Fn()>
		let raw_fn_ptr: *mut dyn Fn(&str) = *recovered_thin_box;  // dereference to get the original fat pointer
		let recovered_closure: Box<dyn Fn(&str)> = unsafe { Box::from_raw(raw_fn_ptr) };  // recreate the original Box<dyn Fn()>
		recovered_closure(&result_str);  // call

	}
	
    pub fn html_element(&self, query: &str, property: &str, value: &str) -> String {  
        let query_cstr = CString::new(query).expect("CString::new failed");
        let property_cstr = CString::new(property).expect("CString::new failed");
        let value_cstr = CString::new(value).expect("CString::new failed");

        let result = unsafe {
            HUI_WebView_html_element(
                self.pointer,
                query_cstr.as_ptr(),
                property_cstr.as_ptr(),
                value_cstr.as_ptr(),
            )
        };

        if result.is_null() {
            String::new()
        } else {
            unsafe { CStr::from_ptr(result).to_string_lossy().into_owned() }
        }
    }

    pub fn call_native<F>(&self, handler: F, process_args: Option<&str>) -> String where F: Fn(Vec<String>) + Send + Sync + 'static + Clone, {  // TODO: find better way to implement default arguments
        let process_args_cstr = CString::new(process_args.unwrap_or("function(...args_array) { return args_array; }")).expect("CString::new failed");

		// store handler in data ptr
		let closure: Box<dyn Fn(Vec<String>)> = Box::new(handler.clone());  // box the closure
		let raw_fn: *mut dyn Fn(Vec<String>) = Box::into_raw(closure);  // convert the Box<dyn Fn()> to a raw pointer
		let thin_box: Box<*mut dyn Fn(Vec<String>)> = Box::new(raw_fn);  // box the fat pointer itself - this makes a thin pointer we can cast safely
		let handler_ptr: *mut c_void = Box::into_raw(thin_box) as *mut c_void;  // convert the Box<*mut dyn Fn()> to *mut c_void
		//let test_string = CString::new("This is a very long test string to check pointer integrity in the callback.").expect("CString::new failed");
        //let handler_ptr = test_string.as_ptr() as *mut c_void;
		println!("Debug: Handler pointer in call_native = {:?}", handler_ptr); //+
		
		
		// store handler globally
        /*let mut global_handler = GLOBAL_HANDLER.lock().unwrap();
        *global_handler = Some(Box::new(handler.clone()));
		println!("Debug: Handler stored in global variable");*/


		// call the function
        let result = unsafe {
            HUI_WebView_call_native(
                self.pointer,
                Some(Self::native_handler),
                //std::ptr::null_mut(), //-
                handler_ptr/*.clone()*/, //+
                process_args_cstr.as_ptr(),
            )
        };
		std::mem::forget(handler);  // no difference
		//std::mem::forget(handler_ptr);  // no difference
		//std::mem::forget(test_string);  // makes the string work


		// return value
        if result.is_null() {
            String::new()
        } 
		else {
            unsafe { CStr::from_ptr(result).to_string_lossy().into_owned() }
        }
    }
    unsafe extern "C" fn native_handler(args: *mut *mut c_char, len: i32, data: *mut c_void) {  // internal handler for js callbacks
		
		// convert C-style arguments to Vec<String>
		let args_vec: Vec<String> = (0..len)
			.map(|i| {
				let cstr = *args.add(i as usize);
				CStr::from_ptr(cstr).to_string_lossy().into_owned()
			})
			.collect();
		println!("Debug: arguments: {:?}", args_vec);
		
		
		// use handler from opaque data pointer
		println!("Debug: handler pointer in native_handler = {:?}", data);
		
		//let test_string = CStr::from_ptr(data as *const c_char);
        //println!("Debug: Received test string: {:?}", test_string.to_string_lossy());
 
		let recovered_thin_box: Box<*mut dyn Fn(Vec<String>)> = unsafe { Box::from_raw(data as *mut *mut dyn Fn(Vec<String>)) };  // cast *mut c_void back to Box<*mut dyn Fn()>
		let raw_fn_ptr: *mut dyn Fn(Vec<String>) = *recovered_thin_box;  // dereference to get the original fat pointer
		let recovered_closure: Box<dyn Fn(Vec<String>)> = unsafe { Box::from_raw(raw_fn_ptr) };  // recreate the original Box<dyn Fn()>
		recovered_closure(args_vec);  // call
	
		std::mem::forget(recovered_thin_box); 
		//std::mem::forget(raw_fn_ptr);  // no difference
		std::mem::forget(recovered_closure); 
		
		
		// use globaly stored handler
        /*let global_handler = GLOBAL_HANDLER.lock().unwrap();  // Retrieve the global handler
        if let Some(handler) = &*global_handler {
            println!("Debug: invoking global handler");
            handler(args_vec);  // call the handler
			//std::mem::forget(data);  // no difference
			//std::mem::forget(handler);  // no difference
        }
		else {
            println!("Debug: no global handler set");
        }*/
		
    } 


    pub fn backend_name(&self) -> String {
        let result = unsafe { HUI_WebView_backend_name(self.pointer) };
        unsafe { CStr::from_ptr(result).to_string_lossy().into_owned() }
    }

    pub fn backend_object(&self) -> *mut c_void {
        unsafe { HUI_WebView_backend_object(self.pointer) }
    }

    pub fn window_type(&self) -> String {
        let result = unsafe { HUI_WebView_window_type(self.pointer) };
        unsafe { CStr::from_ptr(result).to_string_lossy().into_owned() }
    }

    pub fn window_handle(&self) -> *mut c_void {
        unsafe { HUI_WebView_window_handle(self.pointer) }
    }


    pub fn handle_forever() {
        unsafe { HUI_WebView_handle_forever() }
    }

    pub fn handle_once() {
        unsafe { HUI_WebView_handle_once() }
    }

    pub fn handle_threaded() {
        unsafe { HUI_WebView_handle_threaded() }
    }

    pub fn exit() {
        unsafe { HUI_WebView_exit() }
    }

}

impl Drop for WebView {
    fn drop(&mut self) {
        unsafe { HUI_WebView_destroy(self.pointer) }
    }
}

// TODO: this is likely not thread-safe (however it somehow works)
unsafe impl Send for WebView {}
unsafe impl Sync for WebView {}


pub struct WindowControls {
    pointer: *mut c_void,
}

impl WindowControls {
	
    pub fn new(backend: *mut c_void, handle: *mut c_void) -> Self {
        let pointer = unsafe { HUI_WindowControls_create(backend, handle) };
        Self { pointer }
    }

    pub fn set_type(&self, window_type: u32) {
        unsafe { HUI_WindowControls_set_type(self.pointer, window_type) }
    }

    pub fn get_type(&self) -> u32 {
        unsafe { HUI_WindowControls_get_type(self.pointer) }
    }

    pub fn set_layer(&self, layer: u32) {
        unsafe { HUI_WindowControls_set_layer(self.pointer, layer) }
    }

    pub fn get_layer(&self) -> u32 {
        unsafe { HUI_WindowControls_get_layer(self.pointer) }
    }

    pub fn set_geometry(&self, geometry: u32) {
        unsafe { HUI_WindowControls_set_geometry(self.pointer, geometry) }
    }

    pub fn get_geometry(&self) -> u32 {
        unsafe { HUI_WindowControls_get_geometry(self.pointer) }
    }

    pub fn set_opacity(&self, opacity: u8) {
        unsafe { HUI_WindowControls_set_opacity(self.pointer, opacity) }
    }

    pub fn get_opacity(&self) -> u8 {
        unsafe { HUI_WindowControls_get_opacity(self.pointer) }
    }

    pub fn set_id(&self, id: &str) {
        let id_cstr = CString::new(id).expect("CString::new failed");
        unsafe { HUI_WindowControls_set_id(self.pointer, id_cstr.as_ptr()) }
    }

    pub fn get_id(&self) -> String {
        let result = unsafe { HUI_WindowControls_get_id(self.pointer) };
        unsafe { CStr::from_ptr(result).to_string_lossy().into_owned() }
    }

    pub fn set_title(&self, title: &str) {
        let title_cstr = CString::new(title).expect("CString::new failed");
        unsafe { HUI_WindowControls_set_title(self.pointer, title_cstr.as_ptr()) }
    }

    pub fn get_title(&self) -> String {
        let result = unsafe { HUI_WindowControls_get_title(self.pointer) };
        unsafe { CStr::from_ptr(result).to_string_lossy().into_owned() }
    }

    pub fn set_exclusive_zone(&self, zone: i16) {
        unsafe { HUI_WindowControls_set_exclusive_zone(self.pointer, zone) }
    }

    pub fn get_exclusive_zone(&self) -> i16 {
        unsafe { HUI_WindowControls_get_exclusive_zone(self.pointer) }
    }

    pub fn set_focused(&self, activated: bool) {
        unsafe { HUI_WindowControls_set_focused(self.pointer, activated) }
    }

    pub fn get_focused(&self) -> bool {
        unsafe { HUI_WindowControls_get_focused(self.pointer) }
    }
}

impl Drop for WindowControls {
    fn drop(&mut self) {
        unsafe { HUI_WindowControls_destroy(self.pointer) }
    }
}


#[link(name = "libHUI")]  // or HUI if the linker/compiler complains 
extern "C" {
	
    // WebView FFI
    fn HUI_WebView_create() -> *mut c_void;
    fn HUI_WebView_destroy(webview: *mut c_void);
	
    fn HUI_WebView_load_file(webview: *mut c_void, file: *const c_char);
    fn HUI_WebView_load_uri(webview: *mut c_void, uri: *const c_char);
    fn HUI_WebView_load_str(webview: *mut c_void, content: *const c_char);
    fn HUI_WebView_hui_tweaks(webview: *mut c_void);
	
    fn HUI_WebView_call_js(webview: *mut c_void, code: *const c_char, return_data: bool) -> *mut c_char;
	fn HUI_WebView_call_js_async(
        webview: *mut c_void,
        code: *const c_char,
        callback: Option<unsafe extern "C" fn(*const c_char, *mut c_void)>,
        data: *mut c_void,
    );
    fn HUI_WebView_call_native(webview: *mut c_void, handler: Option<unsafe extern "C" fn(*mut *mut c_char, i32, *mut c_void)>, data: *mut c_void, process_args: *const c_char) -> *mut c_char;
    fn HUI_WebView_html_element(webview: *mut c_void, query: *const c_char, property: *const c_char, value: *const c_char) -> *mut c_char;
		
    fn HUI_WebView_backend_name(webview: *mut c_void) -> *const c_char;
    fn HUI_WebView_backend_object(webview: *mut c_void) -> *mut c_void;
    fn HUI_WebView_window_type(webview: *mut c_void) -> *const c_char;
    fn HUI_WebView_window_handle(webview: *mut c_void) -> *mut c_void;
	
    fn HUI_WebView_handle_forever();
    fn HUI_WebView_handle_once();
    fn HUI_WebView_handle_threaded();
    fn HUI_WebView_exit();


    // WindowControls FFI
    fn HUI_WindowControls_create(backend: *mut c_void, handle: *mut c_void) -> *mut c_void;
    fn HUI_WindowControls_destroy(controls: *mut c_void);
	
    fn HUI_WindowControls_set_type(controls: *mut c_void, window_type: u32);
    fn HUI_WindowControls_get_type(controls: *mut c_void) -> u32;
	
    fn HUI_WindowControls_set_layer(controls: *mut c_void, layer: u32);
    fn HUI_WindowControls_get_layer(controls: *mut c_void) -> u32;
	
    fn HUI_WindowControls_set_geometry(controls: *mut c_void, geometry: u32); // TODO: wrong argument type
    fn HUI_WindowControls_get_geometry(controls: *mut c_void) -> u32; // TODO: wrong return type
	
    fn HUI_WindowControls_set_opacity(controls: *mut c_void, opacity: u8);
    fn HUI_WindowControls_get_opacity(controls: *mut c_void) -> u8;
	
    fn HUI_WindowControls_set_id(controls: *mut c_void, id: *const c_char);
    fn HUI_WindowControls_get_id(controls: *mut c_void) -> *const c_char;
	
    fn HUI_WindowControls_set_title(controls: *mut c_void, title: *const c_char);
    fn HUI_WindowControls_get_title(controls: *mut c_void) -> *const c_char;
	
    fn HUI_WindowControls_set_exclusive_zone(controls: *mut c_void, zone: i16);
    fn HUI_WindowControls_get_exclusive_zone(controls: *mut c_void) -> i16;
	
    fn HUI_WindowControls_set_focused(controls: *mut c_void, activated: bool);
    fn HUI_WindowControls_get_focused(controls: *mut c_void) -> bool;
	
	// TODO: missing members + defined values missing + default values

}

