// Test library configuration for windows.cfg
//
// Usage:
// $ cppcheck --check-library --library=windows --enable=information --enable=style --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr test/cfg/windows.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

class CSharedFilesCtrl {
    void OpenFile(const CShareableFile* file, int, int);
    afx_msg void OnNmDblClk(NMHDR *pNMHDR, LRESULT *pResult);

};

void CSharedFilesCtrl::OnNmDblClk(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    if (file)
        OpenFile(file,0,0); // <- not the windows OpenFile function
}
