/*
 * Process Hacker Network Tools -
 *   Ping dialog
 *
 * Copyright (C) 2013 dmex
 *
 * This file is part of Process Hacker.
 *
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nettools.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define WM_PING_UPDATE (WM_APP + 151)
static RECT NormalGraphTextMargin = { 5, 5, 5, 5 };
static RECT NormalGraphTextPadding = { 3, 3, 3, 3 };

static HFONT InitializeFont(
    __in HWND hwndDlg
    )
{
    LOGFONT logFont = { 0 };
    HFONT fontHandle = NULL;

    logFont.lfHeight = 20;
    logFont.lfWeight = FW_MEDIUM;
    logFont.lfQuality = CLEARTYPE_QUALITY | ANTIALIASED_QUALITY;

    wcscpy_s(logFont.lfFaceName, _countof(logFont.lfFaceName),
        WindowsVersion > WINDOWS_XP ? L"Segoe UI" : L"MS Shell Dlg 2"
        );

    fontHandle = CreateFontIndirect(&logFont);

    if (fontHandle)
    {
        SendMessage(hwndDlg, WM_SETFONT, (WPARAM)fontHandle, FALSE);
        return fontHandle;
    }

    return NULL;
}

static NTSTATUS PhPingNetworkPingThreadStart(
    __in PVOID Parameter
    )
{ 
    ULONG icmpReplyLength = 0;
    ULONG icmpReplyCount = 0;
    PVOID icmpReplyBuffer = NULL;
    PICMP_ECHO_REPLY icmpReplyStruct = NULL;
    HANDLE icmpHandle = INVALID_HANDLE_VALUE;
    IP_OPTION_INFORMATION pingOptions = { 0 };

    PNETWORK_OUTPUT_CONTEXT context = (PNETWORK_OUTPUT_CONTEXT)Parameter;

    BOOLEAN isLookupEnabled = !!PhGetIntegerSetting(L"ProcessHacker.NetTools.EnableHostnameLookup");
    ULONG maxPingCount = __max(PhGetIntegerSetting(L"ProcessHacker.NetTools.MaxPingCount"), 1);
    ULONG maxPingTimeout = __max(PhGetIntegerSetting(L"ProcessHacker.NetTools.MaxPingTimeout"), 1);

    __try
    {
        pingOptions.Flags = IP_FLAG_DF;
        pingOptions.Ttl = 128;

        icmpHandle = IcmpCreateFile();
        icmpReplyLength = sizeof(ICMP_ECHO_REPLY);
        icmpReplyBuffer = PhAllocate(icmpReplyLength);
        memset(icmpReplyBuffer, 0, icmpReplyLength);

        context->PingSentCount++;

        icmpReplyCount = IcmpSendEcho(
            icmpHandle,
            context->NetworkItem->RemoteEndpoint.Address.InAddr.S_un.S_addr,
            NULL, 0,
            &pingOptions,
            icmpReplyBuffer,
            icmpReplyLength,
            maxPingTimeout * 1000
            );

        icmpReplyStruct = (PICMP_ECHO_REPLY)icmpReplyBuffer;
        if (icmpReplyCount > 0 && icmpReplyStruct)
        {   
            //Ping statistics for %s:
            //    Packets: Sent = 4, Received = 4, Lost = 0 (0% loss),
            //Approximate round trip times in milli-seconds:
            //    Minimum = 202ms, Maximum = 207ms, Average = 204ms

            //if (icmpReplyStruct->Address == context->Address.InAddr.S_un.S_addr) 
            //if (icmpReplyStruct->DataSize < max_size)
            //if (icmpReplyStruct->RoundTripTime < 1)

            InterlockedIncrement(&context->PingRecvCount);

            if (icmpReplyStruct->Status != IP_SUCCESS)
            {
                InterlockedIncrement(&context->PingLossCount);
            }

            context->CurrentPingMs = icmpReplyStruct->RoundTripTime;

            if (context->CurrentPingMs < context->PingMinMs)
                context->PingMinMs = context->CurrentPingMs;
            if (context->CurrentPingMs > context->PingMaxMs)
                context->PingMaxMs = context->CurrentPingMs;

            PhAddItemCircularBuffer_ULONG(&context->PingHistory, context->CurrentPingMs);   
        }
        else
        {
            InterlockedIncrement(&context->PingLossCount);

            PhAddItemCircularBuffer_ULONG(&context->PingHistory, 0);
        }
    }
    __finally
    {
        if (icmpReplyBuffer)
        {
            PhFree(icmpReplyBuffer);
        }

        if (icmpHandle)
        {
            IcmpCloseHandle(icmpHandle);
        }
    }

    return STATUS_SUCCESS;
}

static VOID NTAPI NetworkPingUpdateHandler(
    __in_opt PVOID Parameter,
    __in_opt PVOID Context
    )
{
    PNETWORK_OUTPUT_CONTEXT context = (PNETWORK_OUTPUT_CONTEXT)Context;
    HANDLE dialogThread = INVALID_HANDLE_VALUE;

    // Queue up the next ping...
    // This needs to be done to prevent slow-links from blocking the interval update.
    if (dialogThread = PhCreateThread(0, (PUSER_THREAD_START_ROUTINE)PhPingNetworkPingThreadStart, (PVOID)context))
        NtClose(dialogThread);

    PostMessage(context->WindowHandle, WM_PING_UPDATE, 0, 0);
}

static INT_PTR CALLBACK NetworkPingWndProc(
    __in HWND hwndDlg,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    )
{
    PNETWORK_OUTPUT_CONTEXT context;

    if (uMsg == WM_INITDIALOG)
    {
        context = (PNETWORK_OUTPUT_CONTEXT)lParam;
        SetProp(hwndDlg, L"Context", (HANDLE)context);
    }
    else
    {
        context = (PNETWORK_OUTPUT_CONTEXT)GetProp(hwndDlg, L"Context");
        if (uMsg == WM_NCDESTROY)
        {
            PhSaveWindowPlacementToSetting(
                L"ProcessHacker.NetTools.NetToolsPingWindowPosition", 
                L"ProcessHacker.NetTools.NetToolsPingWindowSize", 
                hwndDlg
                ); 
            RemoveProp(hwndDlg, L"Context");
        }
    }

    if (!context)
        return FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PH_RECTANGLE windowRectangle;

            // Convert IP Address to string format.
            if (context->NetworkItem->RemoteEndpoint.Address.Type == PH_IPV4_NETWORK_TYPE)
            {
                RtlIpv4AddressToString(
                    &context->NetworkItem->RemoteEndpoint.Address.InAddr, 
                    context->addressString
                    );
            }
            else
            {
                RtlIpv6AddressToString(
                    &context->NetworkItem->RemoteEndpoint.Address.In6Addr, 
                    context->addressString
                    );
            }
           
            SetWindowText(hwndDlg, PhaFormatString(L"Ping (%s)", context->addressString)->Buffer);
            SetWindowText(GetDlgItem(hwndDlg, IDC_MAINTEXT), PhaFormatString(L"Pinging %s with 32 bytes of data:", context->addressString)->Buffer);

            InitializeFont(GetDlgItem(hwndDlg, IDC_MAINTEXT));
            PhInitializeGraphState(&context->PingGraphState);
            PhInitializeLayoutManager(&context->LayoutManager, hwndDlg);
            PhInitializeCircularBuffer_ULONG(&context->PingHistory, PhGetIntegerSetting(L"SampleCount"));
            PhAddLayoutItem(&context->LayoutManager, GetDlgItem(hwndDlg, IDC_CUSTOM1), NULL, PH_ANCHOR_LEFT | PH_ANCHOR_RIGHT | PH_ANCHOR_TOP | PH_ANCHOR_BOTTOM);
            PhAddLayoutItem(&context->LayoutManager, GetDlgItem(hwndDlg, IDC_STATIC3), NULL, PH_ANCHOR_BOTTOM | PH_ANCHOR_LEFT);
            PhAddLayoutItem(&context->LayoutManager, GetDlgItem(hwndDlg, IDC_STATIC5), NULL, PH_ANCHOR_BOTTOM | PH_ANCHOR_LEFT);
            PhAddLayoutItem(&context->LayoutManager, GetDlgItem(hwndDlg, IDC_STATIC6), NULL, PH_ANCHOR_BOTTOM | PH_ANCHOR_LEFT);
            PhAddLayoutItem(&context->LayoutManager, GetDlgItem(hwndDlg, IDC_STATIC7), NULL, PH_ANCHOR_BOTTOM | PH_ANCHOR_LEFT);
            PhAddLayoutItem(&context->LayoutManager, GetDlgItem(hwndDlg, IDC_STATIC8), NULL, PH_ANCHOR_BOTTOM | PH_ANCHOR_LEFT);
            PhAddLayoutItem(&context->LayoutManager, GetDlgItem(hwndDlg, IDOK), NULL, PH_ANCHOR_BOTTOM | PH_ANCHOR_RIGHT);
            Graph_SetTooltip(GetDlgItem(hwndDlg, IDC_CUSTOM1), TRUE);

            context->UseOldColors = !!PhGetIntegerSetting(L"GraphColorMode");
            windowRectangle.Position = PhGetIntegerPairSetting(L"ProcessHacker.NetTools.NetToolsPingWindowPosition");
            windowRectangle.Size = PhGetIntegerPairSetting(L"ProcessHacker.NetTools.NetToolsPingWindowSize");
         
            // Load window settings.
            if (windowRectangle.Position.X == 0 || windowRectangle.Position.Y == 0)
                PhCenterWindow(hwndDlg, GetParent(hwndDlg));
            else
            {
                PhLoadWindowPlacementFromSetting(
                    L"ProcessHacker.NetTools.NetToolsPingWindowPosition", 
                    L"ProcessHacker.NetTools.NetToolsPingWindowSize", 
                    hwndDlg
                    );
            }

            PhRegisterCallback(
                PhGetGeneralCallback(GeneralCallbackProcessesUpdated),
                NetworkPingUpdateHandler,
                context,
                &context->ProcessesUpdatedRegistration
                );
        }
        return TRUE;
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDCANCEL:
            case IDOK:
                PostQuitMessage(0);
                break;
            }
        }
        break;
    case WM_DESTROY:
        {
            PhUnregisterCallback(
                PhGetGeneralCallback(GeneralCallbackProcessesUpdated), 
                &context->ProcessesUpdatedRegistration
                );

            PhDeleteLayoutManager(&context->LayoutManager);
        }
        break;
    case WM_SIZE:
        PhLayoutManagerLayout(&context->LayoutManager);
        break;
    case WM_SIZING:
        PhResizingMinimumSize((PRECT)lParam, wParam, 100, 100);
        break;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
        {
            HDC hDC = (HDC)wParam;
            HWND hwndChild = (HWND)lParam;

            // Check for our static label and change the color.
            if (GetDlgCtrlID(hwndChild) == IDC_MAINTEXT)
            {
                SetTextColor(hDC, RGB(19, 112, 171));
            }

            // Set a transparent background for the control backcolor.
            SetBkMode(hDC, TRANSPARENT);

            // set window background color.
            return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
        }       
        break;
    case WM_PING_UPDATE:
        {
            context->PingGraphState.Valid = FALSE;
            context->PingGraphState.TooltipIndex = -1;
            Graph_MoveGrid(GetDlgItem(hwndDlg, IDC_CUSTOM1), 1);
            Graph_Draw(GetDlgItem(hwndDlg, IDC_CUSTOM1));
            Graph_UpdateTooltip(GetDlgItem(hwndDlg, IDC_CUSTOM1));
            InvalidateRect(GetDlgItem(hwndDlg, IDC_CUSTOM1), NULL, FALSE);
      
            //SetDlgItemText(hwndDlg, IDC_STATIC2, PhaFormatString(
            //    L"Sent: %I64u", context->PingSentCount)->Buffer);
            SetDlgItemText(hwndDlg, IDC_STATIC5, PhaFormatString(
                L"Loss: %I64u (0%% loss)", context->PingLossCount)->Buffer);
            SetDlgItemText(hwndDlg, IDC_STATIC6, PhaFormatString(
                L"Minimum: %I64ums", context->PingMinMs)->Buffer);
            SetDlgItemText(hwndDlg, IDC_STATIC7, PhaFormatString(
                L"Maximum: %I64ums", context->PingMaxMs)->Buffer);
            SetDlgItemText(hwndDlg, IDC_STATIC8, PhaFormatString(
                L"Average: %I64ums", context->PingAvgMs)->Buffer);
        }
        break;
    case WM_NOTIFY:
        {
            LPNMHDR header = (LPNMHDR)lParam;

            switch (header->code)
            {
            case GCN_GETDRAWINFO:
                {
                    PPH_GRAPH_GETDRAWINFO getDrawInfo = (PPH_GRAPH_GETDRAWINFO)header;
                    PPH_GRAPH_DRAW_INFO drawInfo = getDrawInfo->DrawInfo;
                    //drawInfo->Flags = ~PH_GRAPH_USE_GRID;

                    if (context->UseOldColors)
                    {                      
                        drawInfo->BackColor = RGB(0x00, 0x00, 0x00);
                        drawInfo->LineColor1 = PhGetIntegerSetting(L"ColorCpuKernel");
                        drawInfo->LineBackColor1 = PhHalveColorBrightness(drawInfo->LineColor1);
                        drawInfo->LineColor2 = PhGetIntegerSetting(L"ColorCpuUser");
                        drawInfo->LineBackColor2 = PhHalveColorBrightness(drawInfo->LineColor2);
                        drawInfo->GridColor = RGB(0x00, 0x57, 0x00);
                        drawInfo->TextColor = RGB(0x00, 0xff, 0x00);
                        drawInfo->TextBoxColor = RGB(0x00, 0x22, 0x00);
                    }
                    else
                    {              
                        drawInfo->BackColor = RGB(0xef, 0xef, 0xef);
                        drawInfo->LineColor1 = PhHalveColorBrightness(PhGetIntegerSetting(L"ColorCpuKernel"));
                        drawInfo->LineBackColor1 = PhMakeColorBrighter(drawInfo->LineColor1, 125);
                        drawInfo->LineColor2 = PhHalveColorBrightness(PhGetIntegerSetting(L"ColorCpuUser"));
                        drawInfo->LineBackColor2 = PhMakeColorBrighter(drawInfo->LineColor2, 125);
                        drawInfo->GridColor = RGB(0xc7, 0xc7, 0xc7);
                        drawInfo->TextColor = RGB(0x00, 0x00, 0x00);
                        drawInfo->TextBoxColor = RGB(0xe7, 0xe7, 0xe7);
                    }

                    if (header->hwndFrom == GetDlgItem(hwndDlg, IDC_CUSTOM1))
                    {
                        if (PhGetIntegerSetting(L"GraphShowText"))
                        {
                            HDC hdc = Graph_GetBufferedContext(GetDlgItem(hwndDlg, IDC_CUSTOM1));

                            PhSwapReference2(&context->PingGraphState.TooltipText, 
                                PhFormatString(L"Ping: %I64ums", context->CurrentPingMs)
                                );

                            SelectObject(hdc, PhApplicationFont);
                            PhSetGraphText(hdc, drawInfo, &context->PingGraphState.TooltipText->sr,
                                &NormalGraphTextMargin, &NormalGraphTextPadding, PH_ALIGN_TOP | PH_ALIGN_LEFT);
                        }
                        else
                        {
                            drawInfo->Text.Buffer = NULL;
                        }

                        PhGraphStateGetDrawInfo(
                            &context->PingGraphState,
                            getDrawInfo,
                            context->PingHistory.Count
                            );

                        if (!context->PingGraphState.Valid)
                        {
                            ULONG i = 0;

                            for (i = 0; i < drawInfo->LineDataCount; i++)
                            {
                                context->PingGraphState.Data1[i] =
                                    (FLOAT)PhGetItemCircularBuffer_ULONG(&context->PingHistory, i);
                            }

                            // Scale the data.
                            PhxfDivideSingle2U(
                                context->PingGraphState.Data1,
                                (FLOAT)context->PingMaxMs + 5,
                                drawInfo->LineDataCount
                                );

                            context->PingGraphState.Valid = TRUE;
                        }
                    }
                }
                break;
            case GCN_GETTOOLTIPTEXT:
                {
                    PPH_GRAPH_GETTOOLTIPTEXT getTooltipText = (PPH_GRAPH_GETTOOLTIPTEXT)lParam;

                    if (getTooltipText->Index < getTooltipText->TotalCount)
                    {
                        if (header->hwndFrom == GetDlgItem(hwndDlg, IDC_CUSTOM1))
                        {
                            if (context->PingGraphState.TooltipIndex != getTooltipText->Index)
                            {
                                ULONG usage = PhGetItemCircularBuffer_ULONG(
                                    &context->PingHistory, 
                                    getTooltipText->Index
                                    );

                                PhSwapReference2(&context->PingGraphState.TooltipText, PhFormatString(L"Ping: %I64ums", usage));
                            }

                            getTooltipText->Text = context->PingGraphState.TooltipText->sr;
                        }
                    }
                }
                break;
            case GCN_MOUSEEVENT:
                {
                    PPH_GRAPH_MOUSEEVENT mouseEvent = (PPH_GRAPH_MOUSEEVENT)lParam;

                    if (mouseEvent->Message == WM_LBUTTONDBLCLK)
                    {
                        if (header->hwndFrom == GetDlgItem(hwndDlg, IDC_CUSTOM1))
                        {
                            PhShowInformation(hwndDlg, L"Double clicked!");
                        }
                    }
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}

NTSTATUS PhNetworkPingDialogThreadStart(
    __in PVOID Parameter
    )
{
    BOOL result;
    MSG message;
    PH_AUTO_POOL autoPool;
    PNETWORK_OUTPUT_CONTEXT context = (PNETWORK_OUTPUT_CONTEXT)Parameter;

    PhInitializeAutoPool(&autoPool);

    context->WindowHandle = CreateDialogParam(
        (HINSTANCE)PluginInstance->DllBase,
        MAKEINTRESOURCE(IDD_PINGDIALOG),
        PhMainWndHandle,
        NetworkPingWndProc,
        (LPARAM)Parameter
        );

    ShowWindow(context->WindowHandle, SW_SHOW);
    SetForegroundWindow(context->WindowHandle);

    while (result = GetMessage(&message, NULL, 0, 0))
    {
        if (result == -1)
            break;

        if (!IsDialogMessage(context->WindowHandle, &message))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        PhDrainAutoPool(&autoPool);
    }

    PhDeleteAutoPool(&autoPool);
    DestroyWindow(context->WindowHandle);
    PhFree(context);
    return STATUS_SUCCESS;
}