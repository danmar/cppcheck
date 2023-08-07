
// Test library configuration for wxwidgets.cfg
//
// Usage:
// $ ./cppcheck --check-library --library=wxwidgets --enable=style,information --inconclusive --error-exitcode=1 --disable=missingInclude --inline-suppr test/cfg/wxwidgets.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <wx/wx.h>
#include <wx/app.h>
#include <wx/dc.h>
#include <wx/log.h>
#include <wx/filefn.h>
#include <wx/spinctrl.h>
#include <wx/artprov.h>
#include <wx/calctrl.h>
#include <wx/combo.h>
#include <wx/icon.h>
#include <wx/bitmap.h>
#include <wx/dataview.h>
#include <wx/memory.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/regex.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/propgrid/property.h>

void uninitvar_wxRegEx_GetMatch(wxRegEx &obj, size_t *start, size_t *len, size_t index)
{
    size_t s,l;
    size_t *sPtr,*lPtr;
    // cppcheck-suppress uninitvar
    (void)obj.GetMatch(&s,lPtr);
    // TODO cppcheck-suppress uninitvar
    (void)obj.GetMatch(sPtr,&l);
    (void)obj.GetMatch(&s,&l);
    (void)obj.GetMatch(start,len);
    (void)obj.GetMatch(start,len,0);
    (void)obj.GetMatch(start,len,index);
}

#ifdef __VISUALC__
// Ensure no duplicateBreak warning is issued after wxLogApiError() calls.
// This function does not terminate execution.
bool duplicateBreak_wxLogApiError(const wxString &msg, const HRESULT &hr, wxString &str)
{
    if (hr) {
        wxLogApiError(msg,hr);
        str = "fail";
        return false;
    }
    return true;
}
#endif

void argDirection_wxString_ToDouble(const wxString &str)
{
    // No warning is expected. Ensure both arguments are treated
    // as output by library configuration
    double value;
    const bool convOk = str.ToDouble(&value);
    if (convOk && value <= 42.0) {}
}

void argDirection_wxString_ToCDouble(const wxString &str)
{
    // No warning is expected. Ensure both arguments are treated
    // as output by library configuration
    double value;
    const bool convOk = str.ToCDouble(&value);
    if (convOk && value <= 42.0) {}
}

void argDirection_wxTextCtrl_GetSelection(const wxTextCtrl *const textCtrl)
{
    // No warning is expected. Ensure both arguments are treated
    // as output by library configuration
    long start;
    long end;
    textCtrl->GetSelection(&start, &end);
    if (start > 0 && end > 0) {}
}

void useRetval_wxString_MakeCapitalized(wxString &str)
{
    // No warning is expected for
    str.MakeCapitalized();
}

void useRetval_wxString_MakeLower(wxString &str)
{
    // No warning is expected for
    str.MakeLower();
}

void useRetval_wxString_MakeUpper(wxString &str)
{
    // No warning is expected for
    str.MakeUpper();
}

wxString containerOutOfBounds_wxArrayString(void)
{
    wxArrayString a;
    a.Add("42");
    a.Clear();
    // TODO: wxArrayString is defined to be a vector
    // TODO: cppcheck-suppress containerOutOfBounds
    return a[0];
}

int containerOutOfBounds_wxArrayInt(void)
{
    wxArrayInt a;
    a.Add(42);
    a.Clear();
    // TODO: wxArrayString is defined to be a vector
    // TODO: cppcheck-suppress containerOutOfBounds
    return a[0];
}

void ignoredReturnValue_wxDC_GetSize(const wxDC &dc, wxCoord *width, wxCoord *height)
{
    // No warning is expected for
    dc.GetSize(width, height);
    // No warning is expected for
    (void)dc.GetSize();
}

void ignoredReturnValue_wxDC_GetSizeMM(const wxDC &dc, wxCoord *width, wxCoord *height)
{
    // No warning is expected for
    dc.GetSizeMM(width, height);
    // Now warning is expected for
    (void)dc.GetSizeMM();
}

wxSizerItem* invalidFunctionArgBool_wxSizer_Add(wxSizer *sizer, wxWindow * window, const wxSizerFlags &flags)
{
    // No warning is expected for
    return sizer->Add(window,flags);
}

bool invalidFunctionArgBool_wxPGProperty_Hide(wxPGProperty *pg, bool hide, int flags)
{
    // cppcheck-suppress invalidFunctionArgBool
    (void)pg->Hide(hide, true);
    // No warning is expected for
    return pg->Hide(hide, flags);
}

wxTextCtrlHitTestResult nullPointer_wxTextCtrl_HitTest(const wxTextCtrl& txtCtrl, const wxPoint& pos)
{
    // no nullPointer-warning is expected
    return txtCtrl.HitTest(pos, NULL);
}

void validCode()
{
    wxString str = wxGetCwd();
    (void)str;

    wxLogGeneric(wxLOG_Message, "test %d", 0);
    wxLogMessage("test %s", "str");

    wxString translation1 = _("text");
    wxString translation2 = wxGetTranslation("text");
    wxString translation3 = wxGetTranslation("string", "domain");
    (void)translation1;
    (void)translation2;
    (void)translation3;
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

void nullPointer(const wxString &str)
{
    // cppcheck-suppress nullPointer
    wxLogGeneric(wxLOG_Message, (char*)NULL);
    // cppcheck-suppress nullPointer
    wxLogMessage((char*)NULL);

    double *doublePtr = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToDouble(doublePtr);
    double *doublePtr1 = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToCDouble(doublePtr1);

    long * longPtr = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToLong(longPtr);
    long * longPtr1 = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToCLong(longPtr1);

    unsigned long * ulongPtr = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToULong(ulongPtr);
    unsigned long * ulongPtr1 = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToCULong(ulongPtr1);

    long long * longLongPtr = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToLongLong(longLongPtr);

    unsigned long long * ulongLongPtr = NULL;
    // cppcheck-suppress nullPointer
    (void)str.ToULongLong(ulongLongPtr);
}

void nullPointer_wxSizer_Add(wxSizer &sizer, wxWindow *w)
{
    wxWindow * const ptr = 0;
    // @todo cppcheck-suppress nullPointer
    sizer.Add(ptr);
    // No warning shall be issued for
    sizer.Add(w);
}

void uninitvar_wxSizer_Add(wxSizer &sizer, wxWindow *w,wxObject* userData)
{
    int uninit1, uninit2, uninit3;
    // cppcheck-suppress uninitvar
    sizer.Add(w,uninit1);
    // cppcheck-suppress uninitvar
    sizer.Add(w,4,uninit2);
    // cppcheck-suppress uninitvar
    sizer.Add(w,4,2,uninit3,userData);
}

void ignoredReturnValue(const wxString &s)
{
    // cppcheck-suppress ignoredReturnValue
    wxGetCwd();
    // cppcheck-suppress ignoredReturnValue
    wxAtoi(s);
    // cppcheck-suppress ignoredReturnValue
    wxAtol(s);
    // cppcheck-suppress ignoredReturnValue
    wxAtof(s);
}

void invalidFunctionArg(const wxString &str)
{
#if wxUSE_SPINCTRL==1
    extern wxSpinCtrl spinCtrlInstance;
    // cppcheck-suppress invalidFunctionArg
    spinCtrlInstance.SetBase(0);
    // cppcheck-suppress invalidFunctionArg
    spinCtrlInstance.SetBase(5);
#endif

    long l;
    // cppcheck-suppress invalidFunctionArg
    (void)str.ToLong(&l, -1);
    // cppcheck-suppress invalidFunctionArg
    (void)str.ToLong(&l, 1);
    // cppcheck-suppress invalidFunctionArg
    (void)str.ToLong(&l, 37);
}

void uninitvar(wxWindow &w)
{
    wxLogLevel logLevelUninit;
    // cppcheck-suppress constVariable
    char cBufUninit[10];
    const char *pcUninit;
    bool uninitBool;
    // cppcheck-suppress uninitvar
    wxLogGeneric(logLevelUninit, "test");
    // cppcheck-suppress uninitvar
    wxLogMessage(cBufUninit);
    // cppcheck-suppress uninitvar
    wxLogMessage(pcUninit);
    // cppcheck-suppress uninitvar
    w.Close(uninitBool);
}

void uninitvar_wxStaticText(wxStaticText &s)
{
    // no warning
    s.Wrap(-1);
    int uninitInt;
    // cppcheck-suppress uninitvar
    s.Wrap(uninitInt);
}

void uninitvar_wxString_NumberConversion(const wxString &str, const int numberBase)
{
    int uninitInteger1;
    int uninitInteger2;
    int uninitInteger3;
    int uninitInteger4;
    int uninitInteger5;
    int uninitInteger6;
    long l;
    long long ll;
    unsigned long ul;
    unsigned long long ull;

    // cppcheck-suppress uninitvar
    (void)str.ToLong(&l, uninitInteger1);
    // cppcheck-suppress uninitvar
    (void)str.ToLongLong(&ll, uninitInteger2);
    // cppcheck-suppress uninitvar
    (void)str.ToULong(&ul, uninitInteger3);
    // cppcheck-suppress uninitvar
    (void)str.ToULongLong(&ull, uninitInteger4);

    // cppcheck-suppress uninitvar
    (void)str.ToCLong(&l, uninitInteger5);
    // cppcheck-suppress uninitvar
    (void)str.ToCULong(&ul, uninitInteger6);
}

void uninitvar_SetMenuBar(wxFrame * const framePtr, wxMenuBar * const menuBarPtr)
{
    wxMenuBar *menuBar;
    // cppcheck-suppress uninitvar
    framePtr->SetMenuBar(menuBar);
    framePtr->SetMenuBar(menuBarPtr);
}

void uninitvar_wxMenuBarAppend(wxMenuBar * const menuBarPtr, wxMenu * const menuPtr, const wxString &title)
{
    wxMenu *menu;
    // cppcheck-suppress uninitvar
    menuBarPtr->Append(menu, title);
    menuBarPtr->Append(menuPtr, title);
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
                         wxComboCtrl &comboCtrl,
                         wxChar * path)
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

#if wxUSE_DEBUG_CONTEXT==1
    // cppcheck-suppress GetLevelCalled
    // cppcheck-suppress ignoredReturnValue
    wxDebugContext::GetLevel();
    // cppcheck-suppress SetLevelCalled
    wxDebugContext::SetLevel(42);
#endif

    // cppcheck-suppress wxDos2UnixFilenameCalled
    wxDos2UnixFilename(path);

    // cppcheck-suppress wxFileNameFromPathCalled
    // cppcheck-suppress ignoredReturnValue
    wxFileNameFromPath(wxT_2("../test.c"));
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

void wxString_test1(wxString s)
{
    for (int i = 0; i <= s.size(); ++i) {
        // cppcheck-suppress stlOutOfBounds
        s[i] = 'x';
    }
}

void wxString_test2()
{
    wxString s;
    // cppcheck-suppress containerOutOfBounds
    s[1] = 'a';
    s.append("abc");
    s[1] = 'B';
    printf("%s", static_cast<const char*>(s.c_str()));
    wxPrintf("%s", s);
    wxPrintf("%s", s.c_str());
    s.Clear();
}

wxString::iterator wxString_test3()
{
    wxString wxString1;
    wxString wxString2;
    // cppcheck-suppress mismatchingContainers
    for (wxString::iterator it = wxString1.begin(); it != wxString2.end(); ++it)
    {}

    wxString::iterator it = wxString1.begin();
    // cppcheck-suppress returnDanglingLifetime
    return it;
}
