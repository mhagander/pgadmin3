//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// Copyright (C) 2002 - 2003, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// frmHelp.cpp - SQL Help Screen
//
//////////////////////////////////////////////////////////////////////////



// wxWindows headers
#include <wx/wx.h>
#include <wx/image.h>
#include "wx/fs_inet.h"

// App headers
#include "pgAdmin3.h"
#include "frmHelp.h"
#include "frmMain.h"
#include "menu.h"

// Icons
#include "images/pgAdmin3.xpm"
#include "images/connect.xpm"
#include "images/reload.xpm"
#include "images/forward.xpm"
#include "images/back.xpm"




BEGIN_EVENT_TABLE(frmHelp, wxFrame)
    EVT_MENU(MNU_BACK,                  frmHelp::OnBack)
    EVT_MENU(MNU_FORWARD,               frmHelp::OnForward)
    EVT_MENU(MNU_REFRESH,               frmHelp::OnRefresh)
END_EVENT_TABLE();



frmHelp::frmHelp(frmMain *fr) 
: wxFrame(fr, -1, wxT(""))
{
    mainForm=fr;
    SetIcon(wxIcon(pgAdmin3_xpm));

    CreateToolBar();
    wxToolBar *toolBar=GetToolBar();
    wxBitmap barBitmaps[3];
    toolBar->SetToolBitmapSize(wxSize(24, 24));
    barBitmaps[0] = wxBitmap(back_xpm);
    barBitmaps[1] = wxBitmap(forward_xpm);
    barBitmaps[2] = wxBitmap(reload_xpm);
    toolBar->AddTool(MNU_BACK, wxT("Back"), barBitmaps[0], wxT("Back to previous page."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_FORWARD, wxT("Forward"), barBitmaps[1], wxT("Forward to next page."), wxITEM_NORMAL);
    toolBar->AddTool(MNU_REFRESH, wxT("Reload"), barBitmaps[2], wxT("Reload current page."), wxITEM_NORMAL);
    toolBar->Realize();

    CreateStatusBar();
    htmlWindow = new ctlHelpWindow(this);
    htmlWindow->SetRelatedFrame(this, APPNAME_L wxT(" - %s"));
    htmlWindow->SetRelatedStatusBar(0);

    wxPoint pos(settings->Read(wxT("frmHelp/Left"), 100), settings->Read(wxT("frmHelp/Top"), 100));
    wxSize size(settings->Read(wxT("frmHelp/Width"), 400), settings->Read(wxT("frmHelp/Height"), 500));
    CheckOnScreen(pos, size, 200, 150);
    Move(pos);
    SetSize(size);

    CheckToolBar();
};


frmHelp::~frmHelp()
{
    mainForm->RemoveFrame(this);
    settings->Write(wxT("frmHelp/Width"), GetSize().x);
    settings->Write(wxT("frmHelp/Height"), GetSize().y);
    settings->Write(wxT("frmHelp/Left"), GetPosition().x);
    settings->Write(wxT("frmHelp/Top"), GetPosition().y);
}


bool frmHelp::Load(const wxString &page)
{
    if (currentPage.IsEmpty())
        htmlWindow->SetPage(wxT("<html><body>Loading ") + page + wxT("</body></html>"));

    currentPage=page;
    bool loaded=htmlWindow->LoadPage(page);
    if (loaded)
        CheckToolBar();
    return loaded;
}


void frmHelp::CheckToolBar()
{
    wxToolBar *tb=GetToolBar();
    tb->EnableTool(MNU_REFRESH, !currentPage.IsEmpty());
    tb->EnableTool(MNU_BACK, htmlWindow->HistoryCanBack());
    tb->EnableTool(MNU_FORWARD, htmlWindow->HistoryCanForward());
}


void frmHelp::OnLinkClicked(const wxHtmlLinkInfo& link)
{
    wxString newPage=link.GetHref();
    long dp=currentPage.Find(wxT(":"));

    if (newPage.StartsWith(wxT("/")))
    {
        // relative to root
        if (dp < 0)
            currentPage=newPage;
        else
            currentPage=currentPage.Mid(dp+2).AfterFirst('/') + newPage;
    }
    else if (dp < 0)
    {
        // relativ to current page
        currentPage=currentPage.BeforeLast('/') + wxT("/") + newPage;
    }
    else
        currentPage=newPage;

    htmlWindow->LoadPage(currentPage);
    CheckToolBar();
}


void frmHelp::OnRefresh(wxCommandEvent &ev)
{
    htmlWindow->LoadPage(currentPage);
    CheckToolBar();
}


void frmHelp::OnBack(wxCommandEvent &ev)
{
    htmlWindow->HistoryBack();
    currentPage=htmlWindow->GetOpenedPage();
    CheckToolBar();
}


void frmHelp::OnForward(wxCommandEvent &ev)
{
    htmlWindow->HistoryForward();
    currentPage=htmlWindow->GetOpenedPage();
    CheckToolBar();
}




ctlHelpWindow::ctlHelpWindow(frmHelp *frm) 
: wxHtmlWindow(frm) 
{
    helpForm = frm; 
    m_Parser->GetFS()->AddHandler(new wxInternetFSHandler());
}


void ctlHelpWindow::OnLinkClicked(const wxHtmlLinkInfo& link)
{
    helpForm->OnLinkClicked(link);
}
