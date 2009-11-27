﻿/*
 * Process Hacker - 
 *   SAM functions
 *
 * Copyright (C) 2009 wj32
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

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using ProcessHacker.Native.Security;

namespace ProcessHacker.Native.Api
{
    public static partial class Win32
    {
        [DllImport("samlib.dll")]
        public static extern NtStatus SamCloseHandle(
            [In] IntPtr SamHandle
            );

        [DllImport("samlib.dll")]
        public static extern NtStatus SamConnect(
            [In] ref UnicodeString ServerName,
            [Out] out IntPtr ServerHandle,
            [In] SamServerAccess DesiredAccess,
            [In] ref ObjectAttributes ObjectAttributes
            );

        [DllImport("samlib.dll")]
        public static extern NtStatus SamEnumerateDomainsInSamServer(
            [In] IntPtr ServerHandle,
            ref int EnumerationContext,
            [Out] out IntPtr Buffer, // SamSidEnumeration**
            [In] int PreferredMaximumLength,
            [Out] out int CountReturned
            );

        [DllImport("samlib.dll")]
        public static extern NtStatus SamFreeMemory(
            [In] IntPtr Buffer
            );

        [DllImport("samlib.dll")]
        public static extern NtStatus SamLookupDomainInSamServer(
            [In] IntPtr ServerHandle,
            [In] ref UnicodeString Name,
            [Out] out IntPtr DomainId // Sid**
            );

        [DllImport("samlib.dll")]
        public static extern NtStatus SamOpenDomain(
            [In] IntPtr ServerHandle,
            [In] SamDomainAccess DesiredAccess,
            [In] IntPtr DomainId, // Sid*
            [Out] out IntPtr DomainHandle
            );

        [DllImport("samlib.dll")]
        public static extern NtStatus SamQuerySecurityObject(
            [In] IntPtr ObjectHandle,
            [In] SecurityInformation SecurityInformation,
            [Out] out IntPtr SecurityDescriptor // SecurityDescriptor**
            );

        [DllImport("samlib.dll")]
        public static extern NtStatus SamSetSecurityObject(
            [In] IntPtr ObjectHandle,
            [In] SecurityInformation SecurityInformation,
            [In] IntPtr SecurityDescriptor // SecurityDescriptor*
            );

        [DllImport("samlib.dll")]
        public static extern NtStatus SamShutdownSamServer(
            [In] IntPtr ServerHandle
            );
    }
}
