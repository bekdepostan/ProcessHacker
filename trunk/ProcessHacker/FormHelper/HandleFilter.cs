﻿/*
 * Process Hacker
 * 
 * Copyright (C) 2008 Dean
 * Copyright (C) 2008 wj32 
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

using System;
using System.ComponentModel;
using System.Threading;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Collections;

namespace ProcessHacker.FormHelper
{

    public sealed class HandleFilter : AsyncOperation
    {
        public delegate void MatchListViewEvent(ArrayList item);
        public delegate void MatchProgressEvent(int currentValue,int count);
        public event MatchListViewEvent MatchListView;
        public event MatchProgressEvent MatchProgress;
        private string strFilter;
        public HandleFilter(ISynchronizeInvoke isi, string strFilter)
            : base(isi)
        { 
            this.strFilter=strFilter;  
        }       
        protected override void DoWork()
        {
            DoFilter(strFilter);
            if (CancelRequested)
            {
                AcknowledgeCancel();
            }
        }

        private void DoFilter(string strFilter)
        {
            // Stop if cancel
            if (!CancelRequested)
            {                

                Win32.SYSTEM_HANDLE_INFORMATION[] handles = null;
                handles = Win32.EnumHandles();
                Dictionary<int, Win32.ProcessHandle> processHandles = new Dictionary<int, Win32.ProcessHandle>();
                
                for (int i = 0; i < handles.Length; i++)
                {
                    // Check for cancellation here too,
                    // otherwise the user might have to wait for much time                    
                    if (CancelRequested) return;

                    if(i%20==0)
                        OnMatchProgress(i, handles.Length);

                    Win32.SYSTEM_HANDLE_INFORMATION handle = handles[i];

                    CompareHandlerBestNameWithFilterString(processHandles, handle, strFilter);
                    // test Exception 
                    //if (i > 2000) throw new Exception("test");
                }
                OnMatchListView(null);
                foreach (Win32.ProcessHandle phandle in processHandles.Values)
                    phandle.Dispose();
            }  
            
        }
        private void CompareHandlerBestNameWithFilterString(Dictionary<int, Win32.ProcessHandle> processHandles, Win32.SYSTEM_HANDLE_INFORMATION currhandle, string strFilter)
        {
            try
            {
                try
                {
                    if (Win32.GetProcessSessionId(currhandle.ProcessId) != Program.CurrentSessionId)
                        return;
                }
                catch
                {
                    return;
                }

                if (!processHandles.ContainsKey(currhandle.ProcessId))
                    processHandles.Add(currhandle.ProcessId,
                        new Win32.ProcessHandle(currhandle.ProcessId, Win32.PROCESS_RIGHTS.PROCESS_DUP_HANDLE));

                Win32.ObjectInformation info = Win32.GetHandleInfo(processHandles[currhandle.ProcessId], currhandle);

                if (!info.BestName.ToLower().Contains(strFilter.ToLower()))
                    return;

                CallMatchListView(currhandle, info);
            }
            catch
            {
                return;
            }
        }

        private void CallMatchListView(Win32.SYSTEM_HANDLE_INFORMATION handle, Win32.ObjectInformation info)
        {
            ListViewItem item = new ListViewItem();
            item.Name = handle.Handle.ToString();
            item.Text = Program.HackerWindow.ProcessList.Items[handle.ProcessId.ToString()].Text +
                " (" + handle.ProcessId.ToString() + ")";
            item.Tag = handle.ProcessId;
            item.SubItems.Add(new ListViewItem.ListViewSubItem(item, info.TypeName));
            item.SubItems.Add(new ListViewItem.ListViewSubItem(item, info.BestName));
            item.SubItems.Add(new ListViewItem.ListViewSubItem(item, "0x" + handle.Handle.ToString("x")));
            OnMatchListView(item);
        }

        private void OnMatchListView(ListViewItem item)
        {
            lock (this)
            {
                if (item == null)
                {
                    if (ListViewItemContainer.Count>0)
                        FireAsync(MatchListView, ListViewItemContainer);
                    return;
                }
                                
                ListViewItemContainer.Add(item);

                if (ListViewItemContainer.Count > 99)
                {
                    ArrayList items = ListViewItemContainer;
                    // has error?
                    FireAsync(MatchListView, items);
                    ListViewItemContainer.Clear();
                }
            }
        }
        private void OnMatchProgress(int currentValue, int allValue)
        {
            lock (this)
            {
                FireAsync(MatchProgress, currentValue, allValue);
            }
        }
        ArrayList ListViewItemContainer = new ArrayList(100);

    }

}



