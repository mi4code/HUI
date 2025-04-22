// NOW, THIS JUST CODE APPENDED TO WEBVIEW 

struct WindowControls::pImpl {
	GtkWidget** window = NULL;
	void _unshow () {
		
		// get webview
		auto w = gtk_bin_get_child(GTK_BIN(*window));
		
		// free webview
		//g_object_ref(G_OBJECT( gtk_bin_get_child(GTK_BIN(*window)) ));  // not needed, reference exists
		gtk_container_remove(GTK_CONTAINER(*window), GTK_WIDGET( w ));

		// keep old window 
		auto old = *window;

		// new window
		*window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		
		// destroy old window
		gdk_window_destroy (gtk_widget_get_window(old));
		
		// add
		gtk_container_add(GTK_CONTAINER(*window), GTK_WIDGET( w ));
		
		// transparency wouldnt hurt
		gtk_widget_set_app_paintable(GTK_WIDGET(*window), TRUE);
		gtk_widget_set_visual(GTK_WIDGET(*window), gdk_screen_get_rgba_visual(gdk_screen_get_default()));
		
		// TODO: preserve other props (geometry, id, ...)
		
	}
	void _popup () {
		
		// get webview
		auto w = gtk_bin_get_child(GTK_BIN(*window));
		
		// free webview
		//g_object_ref(G_OBJECT( gtk_bin_get_child(GTK_BIN(*window)) ));  // not needed, reference exists
		gtk_container_remove(GTK_CONTAINER(*window), GTK_WIDGET( w ));

		// keep old window 
		auto old = *window;

		// new window
		*window = gtk_window_new(GTK_WINDOW_POPUP);
		
		// destroy old window
		gdk_window_destroy (gtk_widget_get_window(old));
		
		// add
		gtk_container_add(GTK_CONTAINER(*window), GTK_WIDGET( w ));
		
		// transparency wouldnt hurt
		gtk_widget_set_app_paintable(GTK_WIDGET(*window), TRUE);
		gtk_widget_set_visual(GTK_WIDGET(*window), gdk_screen_get_rgba_visual(gdk_screen_get_default()));
		
		gtk_widget_show_all(GTK_WIDGET(*window));
		
		// TODO: preserve other props (geometry, id, ...)
		
	}
};

WindowControls::WindowControls(void* backend, void* handle){
	impl = std::make_unique<pImpl>();
	impl->window = static_cast<GtkWidget**>(backend);
	HUI_DEBUG_PRINT ("controls pointer... "<<*impl->window<<" (validity: "<<GTK_IS_WIDGET(*impl->window)<<")");
	
}
WindowControls::~WindowControls() {
	
}

void WindowControls::set_type (WindowType type){
	// note: when called again for same type as already set, just set it again
	if (type == WT_POPOVER){
		impl->_popup();
		return;
	}
	
	if (type == WT_DESKTOP_COMPONENT and gtk_layer_is_supported ()){
		
		// little hack
		impl->_unshow();
		
		// layer shell
		gtk_layer_init_for_window(GTK_WINDOW(*impl->window)); 
		
		// show it
		gtk_widget_show_all(GTK_WIDGET(*impl->window));
	
		return;
	}
	
	if ( (get_type() == WT_POPOVER) or (gtk_layer_is_supported() and gtk_layer_is_layer_window(GTK_WINDOW(*impl->window))) ){
		// little hack
		impl->_unshow();
		
		// show it
		gtk_widget_show_all(GTK_WIDGET(*impl->window));
	}
	
	// now we have guaranteed normal toplevel window
	
	if (type == WT_DESKTOP_COMPONENT){
		gtk_window_set_decorated(GTK_WINDOW(*impl->window), false);
		gtk_window_set_skip_pager_hint(GTK_WINDOW(*impl->window), true);
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(*impl->window), true);
	}
	else {
		gtk_window_set_decorated(GTK_WINDOW(*impl->window), true);
		gtk_window_set_skip_pager_hint(GTK_WINDOW(*impl->window), false);
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(*impl->window), false);
		
		if (type == WT_TOPLEVEL_SSD){
			gtk_window_set_titlebar(GTK_WINDOW(*impl->window), NULL);
		}
		else if (type == WT_TOPLEVEL_CSD){
			auto placeholder = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
			gtk_widget_set_size_request(placeholder, -1, 0);
			gtk_window_set_titlebar(GTK_WINDOW(*impl->window), placeholder);
		}
		// TODO: decoration scheme
	}	
}
WindowType WindowControls::get_type (){  
	if (gtk_layer_is_supported())  if (gtk_layer_is_layer_window(GTK_WINDOW(*impl->window)))  return WT_DESKTOP_COMPONENT;

	if (gtk_window_get_window_type(GTK_WINDOW(*impl->window)) == GTK_WINDOW_POPUP) return WT_POPOVER;

	if (gtk_window_get_titlebar(GTK_WINDOW(*impl->window)) != NULL) return WT_TOPLEVEL_CSD;
	
    if ( !gtk_window_get_decorated(GTK_WINDOW(*impl->window)) or gtk_window_get_skip_pager_hint(GTK_WINDOW(*impl->window)) or gtk_window_get_skip_taskbar_hint(GTK_WINDOW(*impl->window)) ) return WT_DESKTOP_COMPONENT;

	return WT_TOPLEVEL_SSD;
}

void WindowControls::set_layer(uint8_t layer){
	if (gtk_layer_is_layer_window(GTK_WINDOW(*impl->window))){
		switch(layer){
			case WL_BACKGROUND:
				gtk_layer_set_layer(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_LAYER_BACKGROUND);
				break;
			case WL_BOTTOM:
				gtk_layer_set_layer(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_LAYER_BOTTOM);
				break;
			case WL_TOP:
				gtk_layer_set_layer(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_LAYER_TOP);
				break;
			case WL_OVERLAY:
				gtk_layer_set_layer(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_LAYER_OVERLAY);
				break;
		}
	}
	else {
		switch(layer){
			case WL_BACKGROUND:
				impl->_unshow(); // little hack
				gtk_window_set_type_hint(GTK_WINDOW(*impl->window), GDK_WINDOW_TYPE_HINT_DESKTOP); // only for not-toplevel - works only before show
				gtk_widget_show_all(GTK_WIDGET(*impl->window)); // show it
				gtk_window_set_keep_above(GTK_WINDOW(*impl->window), false); 
				gtk_window_set_keep_below(GTK_WINDOW(*impl->window), false); // should be true of false? - doesnt make a difference ; is there for compatibility ; but doesnt work under windows anyway
				break;
			case WL_TOPLEAST:
				gtk_window_set_keep_below(GTK_WINDOW(*impl->window), true);
				gtk_window_set_keep_above(GTK_WINDOW(*impl->window), false);
				break;
			case WL_TOP:
				gtk_window_set_keep_above(GTK_WINDOW(*impl->window), false);
				gtk_window_set_keep_below(GTK_WINDOW(*impl->window), false);
				break;
			case WL_TOPMOST:
				gtk_window_set_keep_above(GTK_WINDOW(*impl->window), true);
				gtk_window_set_keep_below(GTK_WINDOW(*impl->window), false);
				break;
		}

	}
}
uint8_t WindowControls::get_layer(){
	if (gtk_layer_is_layer_window(GTK_WINDOW(*impl->window))){
		GtkLayerShellLayer layer = gtk_layer_get_layer (GTK_WINDOW(*impl->window));
		switch(layer){
			case GTK_LAYER_SHELL_LAYER_BACKGROUND:
				return WL_BACKGROUND;
				break;
		
			case GTK_LAYER_SHELL_LAYER_BOTTOM:
				return WL_BOTTOM;
				break;
		
			case GTK_LAYER_SHELL_LAYER_TOP:
				return WL_TOP;
				break;
		
			case GTK_LAYER_SHELL_LAYER_OVERLAY:
				return WL_OVERLAY;
				break;
		}
		return WL_TOP;
	}
	else {
		// TODO: not possible to get topmost/topleast values from gtk3
		if (gtk_window_get_type_hint(GTK_WINDOW(*impl->window)) ==  GDK_WINDOW_TYPE_HINT_DESKTOP) return WL_BACKGROUND;
		return WL_TOP;
	}
}

void WindowControls::set_geometry (WindowGeometry geometry){
	// TODO: relative to usable space https://docs.gtk.org/gdk3/method.Monitor.get_workarea.html keep in mind that this doesnt take panels into account as well as invisible areas of the multiscreen rectangle -- there should be arg for this in layer shell
	// TODO: relative to parent toplevel, if no toplevel then workarea/???
	if (gtk_layer_is_layer_window(GTK_WINDOW(*impl->window))){ 
		
		if (geometry.top != -1) {
			gtk_layer_set_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_TOP, true);
			gtk_layer_set_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_TOP, geometry.top);
		}
		else {
			gtk_layer_set_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_TOP, false);
			gtk_layer_set_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_TOP, 0);
		}
		
		if (geometry.left != -1) {
			gtk_layer_set_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_LEFT, true);
			gtk_layer_set_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_LEFT, geometry.left);
		}
		else {
			gtk_layer_set_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_LEFT, false);
			gtk_layer_set_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_LEFT, 0);
		}
		
		if (geometry.right != -1) {
			gtk_layer_set_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_RIGHT, true);
			gtk_layer_set_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_RIGHT, geometry.right);
		}
		else {
			gtk_layer_set_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_RIGHT, false);
			gtk_layer_set_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_RIGHT, 0);
		}
		
		if (geometry.bottom != -1) {
			gtk_layer_set_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_BOTTOM, true);
			gtk_layer_set_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_BOTTOM, geometry.bottom);
		}
		else {
			gtk_layer_set_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_BOTTOM, false);
			gtk_layer_set_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_BOTTOM, 0);
		}
		
		if (geometry.left != -1 and geometry.right != -1) geometry.width = -1;
		if (geometry.top != -1 and geometry.bottom != -1) geometry.height = -1;	
		gtk_widget_set_size_request(GTK_WIDGET(*impl->window), geometry.width, geometry.height); 
		
		if (geometry.monitor >= 0){
			//auto m = gdk_display_get_monitors( gdk_display_manager_get_default_display (gdk_display_manager_get()) ); // GTK4
			//gtk_layer_set_monitor ( GTK_WINDOW(*impl->window), g_list_model_get_item(m, geometry.monitor) ); // GTK4  // TODO: there should be better way to number monitors then by list indexs
			gtk_layer_set_monitor ( GTK_WINDOW(*impl->window), gdk_display_get_monitor ( gdk_display_manager_get_default_display(gdk_display_manager_get()), geometry.monitor) );
			/* TODO: multiscreen monitor (should this be implemented? - for example wayfire shows window only on one monitor at time), OLD CODE BELLOW
				if (relative == -2) {  // recalculate to screen with overflow
					Pixels sw;
					Pixels sh;
					for (uint8_t ss = gdk_screen_get_n_monitors(gdk_screen_get_default())-1; ss>=0; ss--){
						GdkRectangle rect;
						gdk_screen_get_monitor_geometry(
							gdk_screen_get_default(), 
							ss,
							&rect
							);
						sw = std::max(int(sw), rect.x+rect.width);
						sh = std::max(int(sh), rect.y+rect.height);
					}
				
					if (right != -1) {
						w = sw-right-left;
						right = -1;
					}
					if (bottom != -1) {
						h = sh-bottom-top;
						bottom = -1;
					}
					
					relative = gdk_screen_get_monitor_at_point(gdk_screen_get_default(), left, top);
					GdkRectangle rect;
					gdk_screen_get_monitor_geometry(gdk_screen_get_default(), relative, &rect);
					left-=rect.x;
					top-=rect.y;
					
				}
				
				if (relative != -1)  gtk_layer_set_monitor(GTK_WINDOW(window), relative!=-3 ? gdk_display_get_monitor(gdk_display_get_default(), relative) : gdk_display_get_primary_monitor(gdk_display_get_default()) ); // old bugfix says that the window may not show up if this in not the last thing set
				*/
		}
	}
	
	else {
		
		// set position and size
		
		// get screen size and recalculate to relative=-2 and right=bottom=-1
		WindowPixels sw;
		WindowPixels sh;
		if (geometry.monitor == -1) { // default or multiscreen
			for (int8_t ss = gdk_screen_get_n_monitors(gdk_screen_get_default())-1; ss>=0; ss--){
			//for (uint8_t ss = gdk_display_get_n_monitors(gdk_display_get_default())-1; ss>=0; ss--){
				GdkRectangle rect;
				gdk_screen_get_monitor_geometry(
					gdk_screen_get_default(),
					ss,
					&rect
					);
				sw = std::max(int(sw), rect.x+rect.width);
				sh = std::max(int(sh), rect.y+rect.height);
			}
			
		}
		else { // screen
			GdkRectangle rect;
			gdk_screen_get_monitor_geometry(
				gdk_screen_get_default(), 
				geometry.monitor!=-3 ? geometry.monitor : gdk_screen_get_primary_monitor(gdk_screen_get_default()), // TODO: handle primary monitor correctly (this is out of specs)
				&rect
				);
			if (geometry.left != -1) geometry.left+=rect.x;
			if (geometry.top != -1) geometry.top+=rect.y;
			if (geometry.right != -1) geometry.right+=rect.x;
			if (geometry.bottom != -1) geometry.bottom+=rect.y;
			sw = rect.width;
			sh = rect.height;
		}
		// recalculate to x, y
		if (geometry.left != -1 and geometry.right != -1){
			geometry.width = sw-geometry.left-geometry.right;
			geometry.right = -1;
		}
		if (geometry.top != -1 and geometry.bottom != -1){
			geometry.height = sh-geometry.top-geometry.bottom;
			geometry.bottom = -1;
		}
		if (geometry.right != -1){
			int ww,wh;	
			if (geometry.width == -1) gtk_window_get_size(GTK_WINDOW(*impl->window), &ww, &wh);
			else ww = geometry.width;
			geometry.left = sw-geometry.right-ww;
			geometry.right = -1;
		}
		if (geometry.bottom != -1){
			int ww,wh;	
			if (geometry.height == -1) gtk_window_get_size(GTK_WINDOW(*impl->window), &ww, &wh);
			else wh = geometry.height;
			geometry.top = sh-geometry.bottom-wh;
			geometry.bottom = -1;
		}
		// set position
		gtk_window_move(GTK_WINDOW(*impl->window), geometry.left, geometry.top);
		gtk_window_resize(GTK_WINDOW(*impl->window), geometry.width, geometry.height);
		
		
		// set state
		// TODO: only for toplevels (hacks: WT_DESKTOP_COMPONENT fulscreen, multiscreen fullscreen)
		if (geometry.state != 0 and geometry.state != -1){
			
			if (geometry.state == (geometry.state|WS_MAXIMIZED)) gtk_window_maximize(GTK_WINDOW(*impl->window));
			else if (geometry.state == (geometry.state|WS_NOT_MAXIMIZED)) gtk_window_unmaximize (GTK_WINDOW(*impl->window));
			
			if (geometry.state == (geometry.state|WS_FULLSCREEN)) {
				if (geometry.monitor >=0) gtk_window_fullscreen_on_monitor(GTK_WINDOW(*impl->window), gdk_screen_get_default(), geometry.monitor);
				else gtk_window_fullscreen(GTK_WINDOW(*impl->window));
			}
			else if (geometry.state == (geometry.state|WS_NOT_FULLSCREEN)) gtk_window_unfullscreen(GTK_WINDOW(*impl->window));
			
			if (geometry.state == (geometry.state|WS_MINIMIZED)) gtk_window_iconify(GTK_WINDOW(*impl->window));
			else if (geometry.state == (geometry.state|WS_NOT_MINIMIZED)) gtk_window_deiconify(GTK_WINDOW(*impl->window));
			
			if (geometry.state == (geometry.state|WS_HIDDEN)) impl->_unshow();
			else if (geometry.state == (geometry.state|WS_NOT_HIDDEN)) gtk_widget_show_all(GTK_WIDGET(*impl->window));
		
		}
	
	}
	
}
WindowGeometry WindowControls::get_geometry (){  // returns geometry in any valid state (may not match the state set)
	WindowGeometry geometry;
	
	if (gtk_layer_is_layer_window(GTK_WINDOW(*impl->window))){
		
		geometry.state = -1;  // no state for layer-shell
	
		geometry.monitor = gdk_screen_get_monitor_at_window(gdk_screen_get_default(), GDK_WINDOW(*impl->window)); // TODO: deprcated and new gdk_display_get_monitor_at_window doesnt provide a way to get monitor number 
		
		if (gtk_layer_get_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_TOP)) geometry.top = gtk_layer_get_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_TOP);
		else geometry.top = -1;	
		
		if (gtk_layer_get_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_LEFT)) geometry.left = gtk_layer_get_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_LEFT);
		else geometry.left = -1;	
		
		if (gtk_layer_get_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_RIGHT)) geometry.right = gtk_layer_get_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_RIGHT);
		else geometry.right = -1;	
		
		if (gtk_layer_get_anchor(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_BOTTOM)) geometry.bottom = gtk_layer_get_margin(GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_EDGE_BOTTOM);
		else geometry.bottom = -1;
		
		int w,h;
		gtk_widget_get_size_request(GTK_WIDGET(*impl->window), &w, &h);
		
		if (geometry.left == -1 or geometry.right == -1) geometry.width = w;
		if (geometry.top == -1 or geometry.bottom == -1) geometry.height = h;
		
	}
	else {
		
		geometry.state = 0; 
		//geometry.state+=WS_HIDDEN // TODO
		//geometry.state+=WS_NOT_HIDDEN
		//geometry.state+=WS_MINIMIZED // TODO
		//geometry.state+=WS_NOT_MINIMIZED 
		//geometry.state+=WS_FULLSCREEN  // TODO: fullscreen (gtk_window_is_fullscreen(GTK_WINDOW(window)); availible only in Gtk4 -- alternative) - compare screen size and window size
		//geometry.state+=WS_NOT_FULLSCREEN
		if (gtk_window_is_maximized(GTK_WINDOW(*impl->window))) geometry.state+=WS_MAXIMIZED;
		else geometry.state+=WS_NOT_MAXIMIZED;
	
		geometry.monitor = -1; // should mean multiscreen
		
		int w,h,x,y;
		gtk_window_get_size(GTK_WINDOW(*impl->window), &w, &h);
		gtk_window_get_position(GTK_WINDOW(*impl->window), &x, &y);
		
		geometry.width = w;
		geometry.height = h;
		geometry.left = y;
		geometry.top = x;
		geometry.right = -1;
		geometry.bottom = -1;
		
	}
	return geometry;
}

void WindowControls::set_opacity (uint8_t opacity){
	// TODO: this doesnt sync with wayfire 'alpha' plugin
	// TODO: check if we dont need to deal with childs
	gtk_widget_set_opacity(GTK_WIDGET(*impl->window), opacity/100); // sets transparent color for everything (useless here)
}
uint8_t WindowControls::get_opacity (){
	return gtk_widget_get_opacity(GTK_WIDGET(*impl->window))*100;
}

void WindowControls::set_title(std::string title){
	gtk_window_set_title(GTK_WINDOW(*impl->window), title.c_str());
}
std::string WindowControls::get_title(){
	return gtk_window_get_title(GTK_WINDOW(*impl->window));
}

void WindowControls::set_id(std::string id){
	if (gtk_layer_is_layer_window(GTK_WINDOW(*impl->window))) gtk_layer_set_namespace(GTK_WINDOW(*impl->window), id.c_str()); // wayland layer shell
	g_set_prgname (id.c_str()); // should work as wayland app_id but TODO: per window
	gtk_window_set_startup_id(GTK_WINDOW(*impl->window), id.c_str()); // x11
}
std::string WindowControls::get_id(){
	if (gtk_layer_is_layer_window(GTK_WINDOW(*impl->window))) return gtk_layer_get_namespace(GTK_WINDOW(*impl->window));
	return g_get_prgname();
	return ""; // TODO: get startup id
}

void WindowControls::set_exclusive_zone (int16_t zone){
	if (gtk_layer_is_layer_window(GTK_WINDOW(*impl->window))){
		if (zone == -2) gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(*impl->window));
		else gtk_layer_set_exclusive_zone(GTK_WINDOW(*impl->window), zone);  // specs here: <https://github.com/wmww/gtk-layer-shell/blob/5f71546112fd284aced13e7b2391a601204bcacd/protocol/wlr-layer-shell-unstable-v1.xml#L176>
	}
	else {
		// TODO: <https://stackoverflow.com/questions/3859045/preventing-window-overlap-in-gtk>
	}
}
int16_t WindowControls::get_exclusive_zone (){
	//gtk_layer_auto_exclusive_zone_is_enabled(GTK_WINDOW(window)); // useless in fact
	return gtk_layer_get_exclusive_zone(GTK_WINDOW(*impl->window));
}

void WindowControls::set_focused (bool activated){
	if (activated) gtk_window_activate_default(GTK_WINDOW(*impl->window));
	// TODO: deativate (hack: create new invisible window)
}
bool WindowControls::get_focused (){
	return bool( gtk_window_is_active(GTK_WINDOW(*impl->window)) );
}  

void WindowControls::set_input_mode_keyboard (uint8_t mode) {  // TODO: CHECK [before & after] show (?always for non layer-shell)
	if (gtk_layer_is_layer_window(GTK_WINDOW(*impl->window))){
		switch(mode){
			case WIM_ALWAYS:
				gtk_layer_set_keyboard_mode (GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);
				break;
			case WIM_AUTO_WINDOW:
				gtk_layer_set_keyboard_mode (GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);
				break;
			case WIM_NEVER:
				gtk_layer_set_keyboard_mode (GTK_WINDOW(*impl->window), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);
				break;
		}
	}
	else {
		switch(mode){
			case WIM_ALWAYS:
				// TODO: can be callback .. on_unfocus = [](){window.activate()}
				break;
			case WIM_AUTO_WINDOW:
				gtk_window_set_accept_focus (GTK_WINDOW(*impl->window), true);
				break;
			case WIM_NEVER:
				gtk_window_set_accept_focus (GTK_WINDOW(*impl->window), false);
				break;
		}
	}
}
uint8_t WindowControls::get_input_mode_keyboard (){
	if (gtk_layer_is_layer_window(GTK_WINDOW(*impl->window))){
		GtkLayerShellKeyboardMode mode = gtk_layer_get_keyboard_mode (GTK_WINDOW(*impl->window));
		switch(mode){
				case GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE:
					return WIM_ALWAYS;
					break;
		
				case GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND:
					return WIM_AUTO_WINDOW;
					break;
		
				case GTK_LAYER_SHELL_KEYBOARD_MODE_NONE:
					return WIM_NEVER;
					break;
		}
		return WIM_AUTO_WINDOW;
	}
	else {
		if (gtk_window_get_accept_focus(GTK_WINDOW(*impl->window))) return WIM_AUTO_WINDOW;
		else return WIM_NEVER;
		// TODO: WIM_ALWAYS
	}
}

