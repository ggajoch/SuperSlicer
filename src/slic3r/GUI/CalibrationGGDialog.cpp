#include "CalibrationGGDialog.hpp"
#include "I18N.hpp"
#include "libslic3r/Model.hpp"
#include "libslic3r/Utils.hpp"
#include "GLCanvas3D.hpp"
#include "GUI.hpp"
#include "GUI_ObjectList.hpp"
#include "Plater.hpp"
#include "Tab.hpp"
#include <wx/scrolwin.h>
#include <wx/display.h>
#include <wx/file.h>
#include "wxExtensions.hpp"

#if ENABLE_SCROLLABLE
static wxSize get_screen_size(wxWindow* window)
{
    const auto idx = wxDisplay::GetFromWindow(window);
    wxDisplay display(idx != wxNOT_FOUND ? idx : 0u);
    return display.GetClientArea().GetSize();
}
#endif // ENABLE_SCROLLABLE

namespace Slic3r {
namespace GUI {

void CalibrationGGDialog::add_button(void (CalibrationGGDialog::* function)(wxCommandEvent&), 
                                     std::string name,
                                     std::string description) {
    wxButton* bt = new wxButton(this, wxID_FILE1, _(L(name)));
    bt->Bind(wxEVT_BUTTON, function, this);
    bt->SetToolTip(_L(description));
    buttons->Add(bt);
    buttons->AddSpacer(10);
}
void CalibrationGGDialog::create_buttons(wxStdDialogButtonSizer* buttons){
    this->buttons = buttons;
    buttons->SetOrientation(1);

    add_button(&CalibrationGGDialog::hollow_cube, "Hollow cube", "Hollow cube in spiral vase mode");
    add_button(&CalibrationGGDialog::edge_extrusion, "Edge extrusion test", "https://www.thingiverse.com/thing:1622868");
    add_button(&CalibrationGGDialog::pressure_advance, "Pressure advance", "Klipper pressure advance calibration.");
    add_button(&CalibrationGGDialog::basic_retraction, "Basic retraction", "https://www.thingiverse.com/thing:2563909");
    add_button(&CalibrationGGDialog::rocket, "Rocket", "https://www.thingiverse.com/thing:4106816");
    add_button(&CalibrationGGDialog::benchy, "3DBenchy", "https://www.thingiverse.com/thing:763622");
    add_button(&CalibrationGGDialog::engine, "Engine", "https://www.thingiverse.com/thing:4575774");
    add_button(&CalibrationGGDialog::all_in_one, "All in one", "https://www.thingiverse.com/thing:4214436");
}

void CalibrationGGDialog::create_geometry(std::string calibration_path) {
    plat = this->main_frame->plater();
    Model& model = plat->model();
    if (!plat->new_project(L(calibration_path)))
        return;

    GLCanvas3D::set_warning_freeze(true);
    objs_idx = plat->load_files(std::vector<std::string>{
            Slic3r::resources_dir()+"/calibration/gg/"+ calibration_path}, true, false, false);

    assert(objs_idx.size() == 1);
    const DynamicPrintConfig* printConfig = this->gui_app->get_tab(Preset::TYPE_FFF_PRINT)->get_config();
    const DynamicPrintConfig* filamentConfig = this->gui_app->get_tab(Preset::TYPE_FFF_FILAMENT)->get_config();
    const DynamicPrintConfig* printerConfig = this->gui_app->get_tab(Preset::TYPE_PRINTER)->get_config();
    
    this->model = model.objects[objs_idx[0]];

    /// --- translate ---
    const ConfigOptionPoints* bed_shape = printerConfig->option<ConfigOptionPoints>("bed_shape");
    Vec2d bed_size = BoundingBoxf(bed_shape->values).size();
    Vec2d bed_min = BoundingBoxf(bed_shape->values).min;
    this->model->translate({ bed_min.x() + bed_size.x() / 2, bed_min.y() + bed_size.y() / 2, 0 });

    
}
void CalibrationGGDialog::remove_slowdown() {
    const DynamicPrintConfig* filament_config = this->gui_app->get_tab(Preset::TYPE_FFF_FILAMENT)->get_config();
    DynamicPrintConfig new_filament_config = *filament_config; //make a copy

    const ConfigOptionInts* fil_conf = filament_config->option<ConfigOptionInts>("slowdown_below_layer_time");
    ConfigOptionInts* new_fil_conf = new ConfigOptionInts();
    new_fil_conf->default_value = 5;
    new_fil_conf->values = fil_conf->values;
    new_fil_conf->values[0] = 0;
    new_filament_config.set_key_value("slowdown_below_layer_time", new_fil_conf);

    this->gui_app->get_tab(Preset::TYPE_FFF_FILAMENT)->load_config(new_filament_config);
    this->main_frame->plater()->on_config_change(new_filament_config);
    this->gui_app->get_tab(Preset::TYPE_FFF_FILAMENT)->update_dirty();
}

void CalibrationGGDialog::make_hollow() {
    this->model->config.set_key_value("perimeters", new ConfigOptionInt(2));
    this->model->config.set_key_value("fill_density", new ConfigOptionPercent(0));
    this->model->config.set_key_value("top_solid_layers", new ConfigOptionInt(0));
    this->model->config.set_key_value("bottom_solid_layers", new ConfigOptionInt(3));
}

void CalibrationGGDialog::make_spiral_vase() {
    const DynamicPrintConfig* print_config = this->gui_app->get_tab(Preset::TYPE_FFF_PRINT)->get_config();
    DynamicPrintConfig new_print_config = *print_config; //make a copy

    new_print_config.set_key_value("spiral_vase", new ConfigOptionBool(true));

    this->gui_app->get_tab(Preset::TYPE_FFF_PRINT)->load_config(new_print_config);
    this->main_frame->plater()->on_config_change(new_print_config);
    this->gui_app->get_tab(Preset::TYPE_FFF_PRINT)->update_dirty();
}

void CalibrationGGDialog::update() {
    //update plater
    GLCanvas3D::set_warning_freeze(false);
    plat->changed_objects(objs_idx);
    plat->is_preview_shown();
    //update everything, easier to code.
    ObjectList* obj = this->gui_app->obj_list();
    obj->update_after_undo_redo();

    plat->reslice();
}

void CalibrationGGDialog::hollow_cube(wxCommandEvent& event_args) {
    create_geometry("hollow_cube.amf");
    make_spiral_vase();
    update();
}

void CalibrationGGDialog::edge_extrusion(wxCommandEvent& event_args) {
    create_geometry("edge_extrusion_test.amf");
    make_hollow();
    update();
}

void CalibrationGGDialog::pressure_advance(wxCommandEvent& event_args) {
    create_geometry("square_tower.amf");

    make_hollow();

    remove_slowdown();
    this->model->config.set_key_value("perimeter_speed", new ConfigOptionFloat(150));
    this->model->config.set_key_value("external_perimeter_speed", new ConfigOptionFloatOrPercent(150, false));
    
    const DynamicPrintConfig* printerConfig = this->gui_app->get_tab(Preset::TYPE_PRINTER)->get_config();
    DynamicPrintConfig new_printer_config = *printerConfig; //make a copy

    { // start gcode
        auto conf_now = printerConfig->option<ConfigOptionString>("start_gcode")->serialize();
        conf_now += "\\n\\nSET_VELOCITY_LIMIT SQUARE_CORNER_VELOCITY=1 ACCEL=500\\nTUNING_TOWER COMMAND=SET_PRESSURE_ADVANCE PARAMETER=ADVANCE START=0 FACTOR=.005\\n";
        auto config_gcode = new ConfigOptionString();
        config_gcode->deserialize(conf_now, true);
        new_printer_config.set_key_value("start_gcode", config_gcode);
    }
    { // end gcode
        auto conf_now = printerConfig->option<ConfigOptionString>("end_gcode")->serialize();
        conf_now += "\\n\\nRESTART\\n";
        auto config_gcode = new ConfigOptionString();
        config_gcode->deserialize(conf_now, true);
        new_printer_config.set_key_value("end_gcode", config_gcode);
    }
    this->gui_app->get_tab(Preset::TYPE_PRINTER)->load_config(new_printer_config);
    this->main_frame->plater()->on_config_change(new_printer_config);
    this->gui_app->get_tab(Preset::TYPE_PRINTER)->update_dirty();

    update();
}

void CalibrationGGDialog::basic_retraction(wxCommandEvent& event_args) {
    create_geometry("basic_retraction.amf");
    update();
}

void CalibrationGGDialog::rocket(wxCommandEvent& event_args) {
    create_geometry("rocket-remixed.amf");
    make_spiral_vase();
    update();
}

void CalibrationGGDialog::benchy(wxCommandEvent& event_args) {
    create_geometry("benchy.amf");
    update();
}

void CalibrationGGDialog::engine(wxCommandEvent& event_args) {
    create_geometry("engine.amf");
    update();
}

void CalibrationGGDialog::all_in_one(wxCommandEvent& event_args) {
    create_geometry("all_in_one.amf");
    update();
}

} // namespace GUI
} // namespace Slic3r
