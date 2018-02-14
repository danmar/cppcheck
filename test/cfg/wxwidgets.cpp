
// Test library configuration for windows.cfg
//
// Usage:
// $ cppcheck --check-library --library=windows --enable=information --error-exitcode=1 --inline-suppr --suppress=missingIncludeSystem test/cfg/windows.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <wx/app.h>
#include <wx/log.h>
#include <wx/filefn.h>
#include <wx/spinctrl.h>
#include <wx/artprov.h>
#include <wx/calctrl.h>
#include <wx/combo.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/dataview.h>

void validCode()
{
    wxString str = wxGetCwd();

    wxLogGeneric(wxLOG_Message, "test %d", 0);
    wxLogMessage("test %s", "str");

    wxString translation1 = _("text");
    wxString translation2 = wxGetTranslation("text");
    wxString translation3 = wxGetTranslation("string", "domain");
}

#if wxUSE_GUI==1
void validGuiCode()
{
#if wxUSE_SPINCTRL==1
    extern wxSpinCtrl spinCtrlInstance;
    spinCtrlInstance.SetBase(10);
    spinCtrlInstance.SetBase(16);
#endif
}
#endif

void nullPointer()
{
    // cppcheck-suppress nullPointer
    wxLogGeneric(wxLOG_Message, (char*)NULL);
    // cppcheck-suppress nullPointer
    wxLogMessage((char*)NULL);
}

void ignoredReturnValue()
{
    // cppcheck-suppress ignoredReturnValue
    wxGetCwd();
}

void invalidFunctionArg()
{
#if wxUSE_SPINCTRL==1
    extern wxSpinCtrl spinCtrlInstance;
    // cppcheck-suppress invalidFunctionArg
    spinCtrlInstance.SetBase(0);
    // cppcheck-suppress invalidFunctionArg
    spinCtrlInstance.SetBase(5);
#endif
}

void uninitvar()
{
    wxLogLevel logLevelUninit;
    char cBufUninit[10];
    char *pcUninit;
    // cppcheck-suppress uninitvar
    wxLogGeneric(logLevelUninit, "test");
    // cppcheck-suppress uninitvar
    wxLogMessage(cBufUninit);
    // cppcheck-suppress uninitvar
    wxLogMessage(pcUninit);
}

void deprecatedFunctions_wxDataViewCustomRenderer(wxDataViewCustomRenderer &dataViewCustomRenderer, wxPoint cursor, wxRect cell, wxDataViewModel *model, const wxDataViewItem &item, unsigned int col)
{
    // cppcheck-suppress ActivateCalled
    dataViewCustomRenderer.Activate(cell, model, item, col);
    // cppcheck-suppress LeftClickCalled
    dataViewCustomRenderer.LeftClick(cursor, cell, model, item, col);
}

void deprecatedFunctions(wxApp &a,
                         const wxString &s,
                         wxArtProvider *artProvider,
                         wxCalendarCtrl &calenderCtrl,
                         wxComboCtrl &comboCtrl)
{
#ifdef __WXOSX__
    // cppcheck-suppress MacOpenFileCalled
    a.MacOpenFile(s);
#endif

#if wxCHECK_VERSION(3, 1, 0)  // wxWidets-3.1.0 or higher:
    // Some functions are not available anymore in newer versions

    // @todo cppcheck-suppress ShowPopupCalled
    comboCtrl.ShowPopup();
#else
    // cppcheck-suppress InsertCalled
    wxArtProvider::Insert(artProvider);

    // cppcheck-suppress GetTextIndentCalled
    // cppcheck-suppress ignoredReturnValue
    comboCtrl.GetTextIndent();

    // cppcheck-suppress HidePopupCalled
    comboCtrl.HidePopup(true);
    // cppcheck-suppress HidePopupCalled
    comboCtrl.HidePopup(false);
    // cppcheck-suppress HidePopupCalled
    comboCtrl.HidePopup(/*default=false*/);

    // cppcheck-suppress SetTextIndentCalled
    comboCtrl.SetTextIndent(0);
#endif

#if defined(__WXMSW__) || defined(__WXGTK__)
    // EnableYearChange() is not available on these GUI systems
#else
    // cppcheck-suppress EnableYearChangeCalled
    calenderCtrl.EnableYearChange(false);
    // cppcheck-suppress EnableYearChangeCalled
    calenderCtrl.EnableYearChange(true);
    // cppcheck-suppress EnableYearChangeCalled
    calenderCtrl.EnableYearChange(/*default=yes*/);
#endif
}
