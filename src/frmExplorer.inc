


' ========================================================================================
' Position all child windows. Called manually and/or by WM_SIZE
' ========================================================================================
Function PositionExplorerWindows( ByVal HWnd As HWnd ) As LRESULT
   
   Dim pWindow As CWindow Ptr = AfxCWindowPtr(HWnd)
   If pWindow = 0 Then Exit Function
   
   ' Get the entire client area
   Dim As Rect rc
   GetClientRect(HWnd, @rc)

   Dim As HWnd hCtl
   Dim As Long nLeft, nTop, nWidth, nHeight
   
   ' The Explorer label was painted in the top area so start our listbox immediately after that
   nWidth  = pWindow->ScaleX(24)
   nHeight = pWindow->ScaleY(24)
   nTop    = pWindow->ScaleY(8)
   
   hCtl = GetDlgItem(HWND, IDC_FRMEXPLORER_BTNCLOSE)
   SetWindowPos( hCtl, 0, rc.Right - (nWidth*1.2), nTop, nWidth, nHeight, SWP_SHOWWINDOW Or SWP_NOZORDER )

   nTop = nTop + nHeight + pWindow->ScaleY(12)

   hCtl = GetDlgItem(HWND_FRMEXPLORER, IDC_FRMEXPLORER_TREE)
   
   If CBool(gApp.GetDocumentCount = 0) And (gApp.IsProjectActive = False) Then
      ' Ensure that the Recent lists are the most current
      If IsWindowVisible(HWND_FRMRECENT) = 0 Then
         LoadRecentFilesTreeview(HWND_FRMRECENT)
         LoadRecentProjectsTreeview(HWND_FRMRECENT)
      End If
      ShowWindow hCtl, SW_HIDE
      ShowWindow HWND_FRMVSCROLLBAR, SW_HIDE
      SetWindowPos( HWND_FRMRECENT, 0, 0, nTop, rc.right, rc.bottom-nTop, _
                    IIf( gApp.GetDocumentCount, SWP_HIDEWINDOW, SWP_SHOWWINDOW) Or SWP_NOZORDER )
   Else
      ' Project is active to continue to show the Explorer tree regardless of whether 
      ' any files are actually open.
      ShowWindow HWND_FRMRECENT, SW_HIDE

      ' Ensure that the treeview root node is created.
      CreateRootNodeExplorerTreeview
            
      ' Show the vertical scroll bar should it be needed.
      Dim As Long nWidthVScroll = AfxGetWindowWidth( HWND_FRMVSCROLLBAR )
      SetWindowPos( hCtl, 0, 0, nTop, _
                    rc.Right - Iif(IsWindowVisible(HWND_FRMVSCROLLBAR), nWidthVScroll, 0), _
                    rc.bottom-nTop, SWP_SHOWWINDOW Or SWP_NOZORDER )
      if IsWindowVisible(HWND_FRMVSCROLLBAR) Then
         SetWindowPos( HWND_FRMVSCROLLBAR, HWND_TOP, rc.Right - nWidthVScroll, nTop, _
                       nWidthVScroll, rc.bottom-nTop, SWP_SHOWWINDOW )
      End if   
      
   End If   

   Function = 0
End Function
 

' ========================================================================================
' Create the treeview root node should it not already exist
' ========================================================================================
Function CreateRootNodeExplorerTreeview() As Long

   Dim As Hwnd hTree = GetDlgItem(HWND_FRMEXPLORER, IDC_FRMEXPLORER_TREE)
   If gApp.hExplorerRootNode = 0 Then
      gApp.hExplorerRootNode = FF_TreeView_InsertItem (hTree, TVI_ROOT, _
                                          Iif(gApp.IsProjectActive, UCase(gApp.ProjectName), "FILES"), _
                                          0, gidxImageOpened, gidxImageOpened )
      FF_TreeView_SetCheckState( hTree, gApp.hExplorerRootNode, true)
      ' Nodes are added to the treeview via the use of AddFunctionsToExplorerTreeview which
      ' is found in the frmExplorer.inc file.
   End If
   
   function = 0
end function   


' ========================================================================================
' Add the names of all subs/functions belonging to this document to the Explorer treeview
' ========================================================================================
Function AddFunctionsToExplorerTreeview( ByVal pDoc As clsDocument Ptr, ByVal fUpdateNodes As BOOLEAN ) As Long

   If pDoc = 0 Then Exit Function
   
   Dim pList As FUNCTION_TYPE Ptr 
   Dim As CWSTR wzFile = AfxStrPathname("NAMEX", pDoc->DiskFilename)
   Dim As Hwnd hTree   = GetDlgItem(HWND_FRMEXPLORER, IDC_FRMEXPLORER_TREE)
   Dim As HTREEITEM hChild
   Dim As BOOLEAN fAddChildren = False
   
   ' If hNodeExplorer does not exist then we must be adding a new document. If it does
   ' exist then we simply need to update the child nodes with the new pointers to the function list.
   If fUpdateNodes = False Then
      If pDoc->hNodeExplorer = 0 Then
         pDoc->hNodeExplorer = FF_TreeView_InsertItem (hTree, gApp.hExplorerRootNode, wzFile, 0, gidxImageClosed, gidxImageClosed)
         FF_TreeView_SetCheckState( hTree, pDoc->hNodeExplorer, False)
         fAddChildren = True
      End If
   Else
      ' Delete any existing children sub/function nodes and then update with new pointer values
      hChild = TreeView_GetChild( hTree, pDoc->hNodeExplorer )
      Do Until hChild = 0
         TreeView_DeleteItem( hTree, hChild )
         hChild = TreeView_GetChild( hTree, pDoc->hNodeExplorer )
      Loop
      fAddChildren = True
   End If
   
   If fAddChildren Then
      ' Ensure the parent node (filename) has the correct pointer
      pList = pDoc->FnListPtr
      FF_TreeView_SetlParam( hTree, pDoc->hNodeExplorer, Cast(LPARAM, pList) )
      ' Add all the sub/function names in this source file
      pList = pList->pListNext
      Do Until pList = 0
         FF_TreeView_InsertItem (hTree, pDoc->hNodeExplorer, pList->zFnName, Cast(LPARAM, pList), gidxImageCode, gidxImageCode)
         pList = pList->pListNext
      Loop      
      Treeview_SortChildren(hTree, pDoc->hNodeExplorer, 0)
   End If
   
   Function = 0
End Function


' ========================================================================================
' Process WM_SIZE message for window/dialog: frmExplorer
' ========================================================================================
Function frmExplorer_OnSize(ByVal HWnd As HWnd, ByVal state As UINT, ByVal cx As Long, ByVal cy As Long) As LRESULT
   If state <> SIZE_MINIMIZED Then
      ' Position all of the child windows
      PositionExplorerWindows(HWnd)
   End If
   Function = 0
End Function


' ========================================================================================
' Process WM_COMMAND message for window/dialog: frmExplorer
' ========================================================================================
Function frmExplorer_OnCommand(ByVal HWnd As HWnd, ByVal id As Long, ByVal hwndCtl As HWnd, ByVal codeNotify As UINT) As LRESULT

   Select case codeNotify
      case BN_CLICKED
         If id = IDC_FRMEXPLORER_BTNCLOSE Then
            ShowWindow HWND_FRMEXPLORER, SW_HIDE
            frmMain_PositionWindows(HWND_FRMMAIN)
         end if
   end select
                       
   Function = 0
End Function


' ========================================================================================
' Process WM_NOTIFY message for window/dialog: frmExplorer
' ========================================================================================
Function frmExplorer_OnNotify(ByVal HWnd As HWnd, ByVal id As Long, ByVal pNMHDR As NMHDR Ptr) As LRESULT
 
   If (id <> IDC_FRMEXPLORER_TREE) Then Return 0
   
   Dim lpNMTV As NM_TREEVIEW Ptr = Cast(NM_TREEVIEW Ptr, pNMHDR)
   Dim lpTVKD As TV_KEYDOWN  Ptr = Cast(TV_KEYDOWN Ptr, pNMHDR)
   Dim As HTREEITEM hItem
   Dim As BOOLEAN bIsButton
   Dim As HWND hTree 
   Dim As LPARAM lParam
   Dim As Long idxImage 
   Dim pList As FUNCTION_TYPE Ptr
   Dim pDoc As clsDocument Ptr
   Dim ht As TVHITTESTINFO
   Dim As POINT pt
   
   ' Determine what node was clicked on
   Select Case pNMHDR->code 
      
      Case NM_CLICK, NM_RCLICK
      ' If we have clicked on a folder item then issue an expand. Must use HitTest because the 
      ' the current node has not been selected yet.
         hTree = lpNMTV->hdr.hWndFrom
         GetCursorPos(@pt)  
         ScreenToClient(hTree, @pt)
         
         ht.pt = pt 
         Treeview_HitTest(hTree, @ht )
         hItem = ht.hItem
         If hItem Then Treeview_Select( hTree, hItem, TVGN_CARET)

         If hItem Then
            ' If we are clicking on the PROJECT name then expand/collapse the entire treeview list.
            If hItem = Treeview_GetRoot(hTree) Then bIsButton = True
            
            ' Determine if we have clicked on the Expand/Collapse button or on the Filename or a Function name.
            If ht.flags <> TVHT_ONITEMLABEL Then bIsButton = True
            
            ' This is a regular file.
            lParam = FF_TreeView_GetlParam(hTree, hItem)
            pList = Cast(FUNCTION_TYPE Ptr, lParam)
            If pList Then
               If pList->bIsHeader Then
                  If FF_TreeView_GetCheckState( hTree, hItem ) = True Then
                     idxImage = gidxImageOpened
                  Else
                     idxImage = gidxImageClosed
                  End If      
               Else
                  bIsButton = False
                  idxImage = gidxImageCode
               End If      
            End If
            
            If bIsButton = True Then
               If FF_TreeView_GetCheckState( hTree, hItem ) = True Then
                  FF_TreeView_SetCheckState( hTree, hItem, False)
                  idxImage = gidxImageClosed
                  Treeview_Expand(hTree, hItem, TVE_COLLAPSE) 
               Else
                  FF_TreeView_SetCheckState( hTree, hItem, True)
                  idxImage = gidxImageOpened
                  Treeview_Expand(hTree, hItem, TVE_EXPAND) 
               End If
            End If

            ' Set the correct image for the node
            Dim ti As TVITEM
            ti.hItem = hItem                                                 
            ti.mask = TVIF_HANDLE Or TVIF_IMAGE Or TVIF_SELECTEDIMAGE
            ti.iImage = idxImage
            ti.iSelectedImage = idxImage
            TreeView_SetItem(hTree, @ti)
            
            If pNMHDR->code = NM_RCLICK Then
               If hItem <> Treeview_GetRoot(hTree) Then 
                  ' Create the popup menu
                  lParam = FF_TreeView_GetlParam(hTree, hItem)
                  pList = Cast(FUNCTION_TYPE Ptr, lParam)
                  If pList Then
                     ' Select the node and show the document
                     SendMessageW(hTree, WM_LBUTTONDBLCLK, 0, 0)
                     Dim As HMENU hPopupMenu = CreateExplorerContextMenu(pList->pDoc)
                     GetCursorPos @pt
                     TrackPopupMenu(hPopUpMenu, 0, pt.x, pt.y, 0, HWND_FRMMAIN, ByVal Null)
                     DestroyMenu hPopUpMenu
                     Function = True
                  End If
               End If   
            End If
            
         End if
            
         
      Case NM_CUSTOMDRAW
         Dim tvcd As NMTVCUSTOMDRAW Ptr = Cast(NMTVCUSTOMDRAW Ptr, pNMHDR)

         Select Case tvcd->nmcd.dwDrawStage
            Case CDDS_PREPAINT
               Return CDRF_NOTIFYITEMDRAW 

            Case CDDS_ITEMPREPAINT 
               Dim As HFONT hFont = AfxGetWindowFont(GetDlgItem(hWnd, id))
               SelectFont(tvcd->nmcd.hdc, hFont)
               If (tvcd->nmcd.uItemState And ODS_SELECTED) And (GetFocus() = tvcd->nmcd.hdr.hwndFrom) Then
                  tvcd->clrTextBk = BGR(9,71,113)
                  tvcd->clrText   = BGR(255,255,250)
               ElseIf (tvcd->nmcd.uItemState And ODS_SELECTED) Then
                  tvcd->clrTextBk = BGR(63,63,70)
                  tvcd->clrText   = BGR(204,204,204)
               ElseIf (tvcd->nmcd.uItemState And CDIS_HOT) Then
                  tvcd->clrTextBk = BGR(42,45,46)
                  tvcd->clrText   = BGR(204,204,204)
               Else
                  'tvcd->clrTextBk = BGR(37,37,37)
                  tvcd->clrTextBk = BGR(80,80,80)
                  tvcd->clrText   = BGR(204,204,204)
               End If
                                   
               Return CDRF_NEWFONT 

         End Select
      
   End Select
 
   Function = 0
End Function


' ========================================================================================
' Process WM_DESTROY message for window/dialog: frmExplorer
' ========================================================================================
Function frmExplorer_OnDestroy(HWnd As HWnd) As LRESULT
   Dim pWindow As CWindow Ptr = AfxCWindowPtr(hWnd)

   Dim As HFONT hFont
   Dim As HWnd hTree
   hTree = GetDlgItem( hWnd, IDC_FRMEXPLORER_TREE)
   hFont = AfxGetWindowFont(hTree):  DeleteFont(hFont)
   
   Function = 0
End Function



' ========================================================================================
' Processes messages for the subclassed frmExplorer treeview control.
' ========================================================================================
Function frmExplorer_Tree_SubclassProc ( _
                  ByVal HWnd   As HWnd, _                 ' // Control window handle
                  ByVal uMsg   As UINT, _                 ' // Type of message
                  ByVal wParam As WPARAM, _               ' // First message parameter
                  ByVal lParam As LPARAM, _               ' // Second message parameter
                  ByVal uIdSubclass As UINT_PTR, _        ' // The subclass ID
                  ByVal dwRefData As DWORD_PTR _          ' // Pointer to reference data
                  ) As LRESULT

   Dim As HWND hTree 
   Dim As POINT pt
   Dim As HTREEITEM hItem
   Dim As BOOLEAN bIsFolder
   Dim As BOOLEAN bIsExpanded

   ' Convert our ENTER key presses into LBUTTONDBLCLK to process them similarly
   If (uMsg = WM_KEYUP) And (Loword(wParam) = VK_RETURN) Then uMsg = WM_LBUTTONDBLCLK

      
   Select Case uMsg

      Case WM_GETDLGCODE
         ' All keyboard input
         Function = DLGC_WANTALLKEYS
         Exit Function

      Case WM_LBUTTONDBLCLK
         hTree = hWnd
         hItem = Treeview_GetSelection(hTree)
         ' This is a regular file. Preview it in the editor.
         Dim pList As FUNCTION_TYPE Ptr
         Dim pDoc As clsDocument Ptr
         lParam = FF_TreeView_GetlParam(hTree, hItem)
         pList = Cast(FUNCTION_TYPE Ptr, lParam)
         If pList Then
            pDoc = frmMain_OpenFileSafely(HWND_FRMMAIN, _
                                    False, _    ' bIsNewFile
                                    False, _    ' bIsTemplate
                                    True,  _    ' bShowInTab
                                    false, _    ' bIsInclude
                                    pList->pDoc->DiskFilename, _  ' pwszName
                                    pList->pDoc )  ' pDocIn
            ' If this is a header node (File name) then set the parent node so that it can be
            ' easily set later when top tab control tabs are clicked.
            If pList->bIsHeader Then
               pDoc->hNodeExplorer = hItem
            Else
               ' This is a function within the file. Attempt to position to the function line.
               ' Set the top line to display in the editor. I chose to start 3 lines before the
               ' function just to make it visually more appealing.
               SendMessageW( pDoc->hWindow, SCI_SETFIRSTVISIBLELINE, pList->nLineNumber - 3, 0) 
               SendMessageW( pDoc->hWindow, SCI_GOTOLINE, pList->nLineNumber, 0) 
            End If      
         End If

         Exit Function
         
      Case WM_KEYUP
         Select Case Loword(wParam)
            Case VK_RETURN  ' already processed in WM_LBUTTONDBLCLK 
         End Select
         Exit Function

      Case WM_CHAR   ' prevent the annoying beep!
         If wParam = VK_RETURN Then Return 0

      Case WM_MOUSELEAVE
         RemovePropW HWND_FRMVSCROLLBAR, "HOT"
         ShowWindow HWND_FRMVSCROLLBAR, SW_HIDE
         PositionExplorerWindows(HWND_FRMEXPLORER)
         
      Case WM_MOUSEMOVE
         ' Display the horizontal scroll bar if we hovering over the bottom of the window.
         Dim rc As rect
         GetClientRect Hwnd, @rc
         
         Dim xpos as long = LOWORD(lParam)
         Dim ypos as long = HIWORD(lParam)
        
         'Dim As Long nHeight = AfxGetWindowHeight( HWND_FRMHSCROLLBAR )

         ' Are we over the scrollbar area
         'If (ypos > rc.bottom - nHeight) Then
            If IsWindowVisible(HWND_FRMVSCROLLBAR) = 0 Then
               ShowWindow(HWND_FRMVSCROLLBAR, SW_SHOW)
               PositionExplorerWindows(HWND_FRMEXPLORER)
               Exit Function
            End If   
         'End If   

      Case WM_DESTROY
         ' REQUIRED: Remove control subclassing
         RemoveWindowSubclass HWnd, @frmExplorer_Tree_SubclassProc, uIdSubclass

   End Select

   ' Default processing of Windows messages
   Function = DefSubclassProc(HWnd, uMsg, wParam, lParam)

End Function


' ========================================================================================
' frmExplorer Window procedure
' ========================================================================================
Function frmExplorer_WndProc( ByVal HWnd   As HWnd, _
                              ByVal uMsg   As UINT, _
                              ByVal wParam As WPARAM, _
                              ByVal lParam As LPARAM _
                              ) As LRESULT

   Select Case uMsg
      HANDLE_MSG (HWnd, WM_SIZE,    frmExplorer_OnSize)
      HANDLE_MSG (HWnd, WM_COMMAND, frmExplorer_OnCommand)
      HANDLE_MSG (HWnd, WM_NOTIFY,  frmExplorer_OnNotify)
      HANDLE_MSG (HWnd, WM_DESTROY, frmExplorer_OnDestroy)
   End Select

   ' for messages that we don't deal with
   Function = DefWindowProcW(HWnd, uMsg, wParam, lParam)

End Function


' ========================================================================================
' frmExplorer_Show
' ========================================================================================
Function frmExplorer_Show( ByVal hWndParent As HWnd ) As LRESULT

   '  Create the main window and child controls
   Dim pWindow As CWindow Ptr = New CWindow
   pWindow->DPI = AfxCWindowPtr(hwndParent)->DPI

   ' Only make the Explorer initially visable if it was already visible
   ' when the most previous instance of the program closed.
   
   HWND_FRMEXPLORER = pWindow->Create( hWndParent, "", @frmExplorer_WndProc, 0, 0, 250, 0, _
        WS_CHILD Or iif(gConfig.ShowExplorer, WS_VISIBLE, 0) Or WS_CLIPSIBLINGS Or WS_CLIPCHILDREN, _
        WS_EX_CONTROLPARENT Or WS_EX_LEFT Or WS_EX_LTRREADING Or WS_EX_RIGHTSCROLLBAR)

   ' Disable background erasing by only assigning the one style
   pWindow->ClassStyle = CS_DBLCLKS
   pWindow->SetFont( "Segoe UI Semibold", 10)
   'pWindow->Brush = CreateSolidBrush(BGR(37,37,37))
   pWindow->Brush = CreateSolidBrush(BGR(80,80,80))

   Dim As HWnd hBtn = _
   frmHotTxtBtn( HWND_FRMEXPLORER, IDC_FRMEXPLORER_LBLEXPLORER, L(247, "EXPLORER"), "", _
                    BGR(204,204,204), BGR(80,80,80), _
                    BGR(204,204,204), BGR(80,80,80), _
                    10, 0, 12, 75, 12 )            ' 0 Left, 1 Center, 2 Right
   AfxSetWindowFont( hBtn, pWindow->Font, True )

   frmHotImgBtn( HWND_FRMEXPLORER, IDC_FRMEXPLORER_BTNCLOSE, "IMAGE_CLOSE2", "IMAGE_CLOSE2",  _
                  L(161, "Close"), 24, 24, BGR(80,80,80), 0, 0, 24, 24 )

   Dim As HWnd hTree = _
        pWindow->AddControl("TREEVIEW", , IDC_FRMEXPLORER_TREE, "", 0, 0, 0, 0, _
        WS_CHILD Or WS_VISIBLE Or WS_TABSTOP Or _
        TVS_NOSCROLL Or TVS_SHOWSELALWAYS Or TVS_FULLROWSELECT Or TVS_TRACKSELECT, _
        WS_EX_LEFT Or WS_EX_RIGHTSCROLLBAR, , _
        Cast(WNDPROC, @frmExplorer_Tree_SubclassProc), IDC_FRMEXPLORER_TREE, Cast(DWORD_PTR, @pWindow))
   
  'Treeview_SetIndent(hTree, 14)     '<-- do not scale this number
   
   Dim As HFONT hFont = pWindow->CreateFont("Segoe UI", 10) 
   SendMessageW( hTree, WM_SETFONT, Cast(WPARAM, hFont), CTRUE )
   'SendMessageW( hTree, TVM_SETBKCOLOR, 0, Cast(LPARAM, BGR(37,37,37)) )
   SendMessageW( hTree, TVM_SETBKCOLOR, 0, Cast(LPARAM, BGR(80,80,80)) )
   SendMessageW( hTree, TVM_SETITEMHEIGHT, pWindow->ScaleY(24), 0 )
   SendMessageW( hTree, TVM_SETEXTENDEDSTYLE, TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER)   
   TreeView_SetImageList( hTree, ghImageListNormal, TVSIL_NORMAL )


   PositionExplorerWindows(HWND_FRMEXPLORER)

   Function = 0
   
End Function




























































