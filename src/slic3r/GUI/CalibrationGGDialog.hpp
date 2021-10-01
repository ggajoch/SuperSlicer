#ifndef slic3r_GUI_CalibrationGGDialog_hpp_
#define slic3r_GUI_CalibrationGGDialog_hpp_

#include "CalibrationAbstractDialog.hpp"

namespace Slic3r { 
namespace GUI {

class CalibrationGGDialog : public CalibrationAbstractDialog
{

public:
    CalibrationGGDialog(GUI_App* app, MainFrame* mainframe) : CalibrationAbstractDialog(app, mainframe, "GG calibrations") { create("/calibration/gg", "gg.html"); }
    virtual ~CalibrationGGDialog(){ }
    
protected:
    std::vector<size_t> objs_idx;
    Plater* plat;
    ModelObject* model;
    wxStdDialogButtonSizer* buttons;

    void add_button(void (CalibrationGGDialog::* function)(wxCommandEvent&), std::string name, std::string description);

    void create_buttons(wxStdDialogButtonSizer* sizer) override;
    void create_geometry(std::string cube_path);
    
    void hollow_cube(wxCommandEvent& event_args);
    void edge_extrusion(wxCommandEvent& event_args);
    void pressure_advance(wxCommandEvent& event_args);
    void basic_retraction(wxCommandEvent& event_args);    
    void rocket(wxCommandEvent& event_args);
    void benchy(wxCommandEvent& event_args);
    void engine(wxCommandEvent& event_args);
    void all_in_one(wxCommandEvent& event_args);


    void remove_slowdown();
    void make_hollow();
    void make_spiral_vase();

    void update();
};

} // namespace GUI
} // namespace Slic3r

#endif
