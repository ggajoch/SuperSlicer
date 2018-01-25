#include "TabIface.hpp"
#include "Tab.hpp"

namespace Slic3r {

void	TabIface::load_current_preset()		{ m_tab->load_current_preset(); }
void	TabIface::update_tab_ui()			{ m_tab->update_tab_ui(); }
void	TabIface::update_ui_from_settings()	{ m_tab->update_ui_from_settings();}
void	TabIface::select_preset(char* name)	{ m_tab->select_preset(name);}
char*	TabIface::title()					{ return (char*)m_tab->title().ToStdString().data();}
void	TabIface::load_config(DynamicPrintConfig* config)	{ m_tab->load_config(*config);}
bool	TabIface::current_preset_is_dirty()					{ return m_tab->current_preset_is_dirty();}
DynamicPrintConfig*			TabIface::get_config()			{ return m_tab->get_config();}
PresetCollection*			TabIface::get_presets()			{ return m_tab->get_presets(); }
std::vector<std::string>	TabIface::get_dependent_tabs()	{ return m_tab->get_dependent_tabs(); }

}; // namespace Slic3r
