// NOW, THIS JUST CODE APPENDED TO WEBVIEW 

struct WindowControls::pImpl {};

WindowControls::WindowControls(void* backend, void* handle) {
    std::cout<<"WindowControls::WindowControls"<<"\n\t"<<backend<<"\n\t"<<handle<<"\n";
}
WindowControls::~WindowControls() {
    std::cout<<"WindowControls::~WindowControls"<<"\n";
}

void WindowControls::set_type(WindowType type) {
    std::cout<<"WindowControls::set_type"<<"\n\t"<<type<<"\n";
}
WindowType WindowControls::get_type() {
    std::cout<<"WindowControls::get_type"<<"\n";
    return WT_TOPLEVEL_SSD;
}

void WindowControls::set_layer(WindowLayer layer) {
    std::cout<<"WindowControls::set_layer"<<"\n\t"<<layer<<"\n";
}
WindowLayer WindowControls::get_layer() {
    std::cout<<"WindowControls::get_layer"<<"\n";
    return WL_TOP;
}

void WindowControls::set_geometry(WindowGeometry geometry) {
    std::cout<<"WindowControls::set_geometry"<<"\n\t"<<geometry.state<<"\n\t"<<geometry.monitor<<"\n\t"<<geometry.width<<"\n\t"<<geometry.height<<"\n\t"<<geometry.left<<"\n\t"<<geometry.top<<"\n\t"<<geometry.right<<"\n\t"<<geometry.bottom<<"\n";
}
WindowGeometry WindowControls::get_geometry() {
    std::cout<<"WindowControls::get_geometry"<<"\n";
    return {};
}

void WindowControls::set_opacity(uint8_t opacity) {
    std::cout<<"WindowControls::set_opacity"<<"\n\t"<<opacity<<"\n";
}
uint8_t WindowControls::get_opacity() {
    std::cout<<"WindowControls::get_opacity"<<"\n";
    return 100;
}

void WindowControls::set_id(std::string id) {
    std::cout<<"WindowControls::set_id"<<"\n\t"<<id<<"\n";
}
std::string WindowControls::get_id() {
    std::cout<<"WindowControls::get_id"<<"\n";
    return "";
}

void WindowControls::set_title(std::string title) {
    std::cout<<"WindowControls::set_title"<<"\n\t"<<title<<"\n";
}
std::string WindowControls::get_title() {
    std::cout<<"WindowControls::get_title"<<"\n";
    return "";
}

void WindowControls::set_exclusive_zone(int16_t zone) {
    std::cout<<"WindowControls::set_exclusive_zone"<<"\n\t"<<zone<<"\n";
}
int16_t WindowControls::get_exclusive_zone() {
    std::cout<<"WindowControls::get_exclusive_zone"<<"\n";
    return 0;
}

void WindowControls::set_focused(bool activated) {
    std::cout<<"WindowControls::set_focused"<<"\n\t"<<activated<<"\n";
}
bool WindowControls::get_focused() {
    std::cout<<"WindowControls::get_focused"<<"\n";
    return false;
}

void WindowControls::set_input_mode_keyboard(WindowInputMode mode) {
    std::cout<<"WindowControls::set_input_mode_keyboard"<<"\n\t"<<mode<<"\n";
}
WindowInputMode WindowControls::get_input_mode_keyboard() {
    std::cout<<"WindowControls::get_input_mode_keyboard"<<"\n";
    return 0;
}
