// NOW, THIS JUST CODE APPENDED TO WEBVIEW 

struct WindowControls::pImpl {
	QWidget** window = NULL;
};

WindowControls::WindowControls(void* backend, void* handle) {
	impl = std::make_unique<pImpl>();
	impl->window = static_cast<QWidget**>(backend);
}
WindowControls::~WindowControls() {

}

void WindowControls::set_type(WindowType type) {
    if (type == WT_TOPLEVEL_SSD or type == WT_TOPLEVEL_CSD) (*impl->window)->setWindowFlags(Qt::Window);
	else (*impl->window)->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
}
WindowType WindowControls::get_type() {
	if ( (*impl->window)->windowFlags() & Qt::FramelessWindowHint ) return WT_DESKTOP_COMPONENT;
    return WT_TOPLEVEL_SSD;
}

void WindowControls::set_layer(WindowLayer layer){

	bool qt_window_shown = (*impl->window)->isVisible(); // the information has to be obtained before any changes are made
	
	switch(layer){
		case WL_BACKGROUND:
			(*impl->window)->setWindowFlags( ((*impl->window)->windowFlags() & (!Qt::WindowStaysOnTopHint) ) | Qt::WindowStaysOnBottomHint );
			(*impl->window)->setAttribute(Qt::WA_X11NetWmWindowTypeDesktop);
			break;
		case WL_TOPLEAST:
			(*impl->window)->setWindowFlags( ((*impl->window)->windowFlags() & (!Qt::WindowStaysOnTopHint) ) | Qt::WindowStaysOnBottomHint );
			break;
		case WL_TOP:
			(*impl->window)->setWindowFlags( ((*impl->window)->windowFlags() & (!Qt::WindowStaysOnTopHint) ) & (!Qt::WindowStaysOnBottomHint) );
			break;
		case WL_TOPMOST:
			(*impl->window)->setWindowFlags( ((*impl->window)->windowFlags() & (!Qt::WindowStaysOnBottomHint) ) | Qt::WindowStaysOnTopHint );
			break;
	}
	
	if (qt_window_shown) (*impl->window)->show(); // required when after show
}
WindowLayer WindowControls::get_layer(){
	// return WL_BACKGROUND // TODO
	if ( !((*impl->window)->windowFlags() & Qt::WindowStaysOnTopHint) and !((*impl->window)->windowFlags() & Qt::WindowStaysOnBottomHint) ) return WL_TOP;
	else if ( ((*impl->window)->windowFlags() & Qt::WindowStaysOnTopHint) and !((*impl->window)->windowFlags() & Qt::WindowStaysOnBottomHint) ) return WL_TOPMOST;
	else if ( !((*impl->window)->windowFlags() & Qt::WindowStaysOnTopHint) and ((*impl->window)->windowFlags() & Qt::WindowStaysOnBottomHint) ) return WL_TOPLEAST;
}

void WindowControls::set_geometry(WindowGeometry geometry) {
    int sw, sh;
    QRect screenRect;
    if (geometry.monitor == -1) {
        QRect unionRect = QApplication::desktop()->screenGeometry();
        sw = unionRect.width();
        sh = unionRect.height();
    } 
	/*else {  // TODO: screens
        QScreen *screen = (geometry.monitor != -3) ? QGuiApplication::screens().value(geometry.monitor) : QGuiApplication::primaryScreen();
        screenRect = screen->geometry();
        if (geometry.left != -1)
            geometry.left += screenRect.x();
        if (geometry.top != -1)
            geometry.top += screenRect.y();
        if (geometry.right != -1)
            geometry.right += screenRect.x();
        if (geometry.bottom != -1)
            geometry.bottom += screenRect.y();
        sw = screenRect.width();
        sh = screenRect.height();
    }*/
    if (geometry.left != -1 && geometry.right != -1) {
        geometry.width = sw - geometry.left - geometry.right;
        geometry.right = -1;
    }
    if (geometry.top != -1 && geometry.bottom != -1) {
        geometry.height = sh - geometry.top - geometry.bottom;
        geometry.bottom = -1;
    }
    if (geometry.right != -1) {
        int ww = (geometry.width == -1) ? (*impl->window)->width() : geometry.width;
        geometry.left = sw - geometry.right - ww;
        geometry.right = -1;
    }
    if (geometry.bottom != -1) {
        int hh = (geometry.height == -1) ? (*impl->window)->height() : geometry.height;
        geometry.top = sh - geometry.bottom - hh;
        geometry.bottom = -1;
    }
    (*impl->window)->setGeometry(geometry.left, geometry.top, geometry.width, geometry.height);
    if (geometry.state != 0 && geometry.state != -1) {
        if (geometry.state == (geometry.state | WS_MAXIMIZED))
            (*impl->window)->showMaximized();
        else if (geometry.state == (geometry.state | WS_NOT_MAXIMIZED))
            (*impl->window)->showNormal();
        if (geometry.state == (geometry.state | WS_FULLSCREEN))
            (*impl->window)->showFullScreen();
        else if (geometry.state == (geometry.state | WS_NOT_FULLSCREEN))
            (*impl->window)->showNormal();
        if (geometry.state == (geometry.state | WS_MINIMIZED))
            (*impl->window)->showMinimized();
        else if (geometry.state == (geometry.state | WS_NOT_MINIMIZED))
            (*impl->window)->showNormal();
        if (geometry.state == (geometry.state | WS_HIDDEN))
            (*impl->window)->hide();
        else if (geometry.state == (geometry.state | WS_NOT_HIDDEN))
            (*impl->window)->show();
    }
}
WindowGeometry WindowControls::get_geometry() {
    WindowGeometry geometry;
    geometry.state = 0;
    if ((*impl->window)->isHidden())
        geometry.state |= WS_HIDDEN;
    else
        geometry.state |= WS_NOT_HIDDEN;
    if ((*impl->window)->isMinimized())
        geometry.state |= WS_MINIMIZED;
    else
        geometry.state |= WS_NOT_MINIMIZED;
    if ((*impl->window)->isFullScreen())
        geometry.state |= WS_FULLSCREEN;
    else
        geometry.state |= WS_NOT_FULLSCREEN;
    if ((*impl->window)->isMaximized())
        geometry.state |= WS_MAXIMIZED;
    else
        geometry.state |= WS_NOT_MAXIMIZED;
    geometry.monitor = -1;
    QRect rect = (*impl->window)->geometry();
    geometry.width = rect.width();
    geometry.height = rect.height();
    geometry.left = rect.x();
    geometry.top = rect.y();
    geometry.right = -1;
    geometry.bottom = -1;
    return geometry;
}

void WindowControls::set_opacity(uint8_t opacity) {
	(*impl->window)->setWindowOpacity(opacity/100);
}
uint8_t WindowControls::get_opacity() {
    std::cout<<"WindowControls::get_opacity"<<"\n";
    return (*impl->window)->windowOpacity()*100;
}

void WindowControls::set_id(std::string id) {
    // TODO
}
std::string WindowControls::get_id() {
    // TODO
    return "";
}

void WindowControls::set_title(std::string title){
	(*impl->window)->setWindowTitle(title.c_str());
}
std::string WindowControls::get_title(){
	return (*impl->window)->windowTitle().toStdString();
}

void WindowControls::set_exclusive_zone(int16_t zone) {
    // TODO
}
int16_t WindowControls::get_exclusive_zone() {
    // TODO
    return 0;
}

void WindowControls::set_focused(bool activated) {
    if(activated) (*impl->window)->activateWindow();
}
bool WindowControls::get_focused() {
    return (*impl->window)->hasFocus();
}

void WindowControls::set_input_mode_keyboard (WindowInputMode mode) {

	bool qt_window_shown = (*impl->window)->isVisible(); 
	
	switch(mode){
		case WIM_ALWAYS:
			/// TODO
			break;
		case WIM_AUTO_WINDOW:
			(*impl->window)->setWindowFlags( (*impl->window)->windowFlags() & (!Qt::WindowDoesNotAcceptFocus) );
			break;
		case WIM_NEVER:
			(*impl->window)->setWindowFlags( (*impl->window)->windowFlags() | Qt::WindowDoesNotAcceptFocus );
			break;
	}
	
	if (qt_window_shown) (*impl->window)->show();

}
WindowInputMode WindowControls::get_input_mode_keyboard(){
	if ((*impl->window)->windowFlags() & Qt::WindowDoesNotAcceptFocus) return WIM_NEVER;
	else return WIM_AUTO_WINDOW;
	// return WIM_ALWAYS // TODO
}
