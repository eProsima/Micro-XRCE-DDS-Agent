# Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

!include x64.nsh

Function InstallRedistributables

    # Check if it is necessary to install to redistributables.
    ClearErrors
    ${If} ${RunningX64}
        SetRegView 64
    ${EndIf}

    ${If} ${SelectionIsSelected} ${libraries_x64Win64VS2017}
        ReadRegStr $0 HKLM "SOFTWARE\Classes\Installer\Dependencies\,,amd64,14.0,bundle" "Version"
        StrCmp $0 "14.15.26706.0" not_install_redist install_redist
#    ${ElseIf} ${SelectionIsSelected} ${libraries_i86Win32VS2017}
        ReadRegStr $0 HKLM "SOFTWARE\Classes\Installer\Dependencies\,,x86,14.0,bundle" "Version"
        StrCmp $0 "14.15.26706.0" not_install_redist install_redist
    ${Else}
    ${EndIf}


    install_redist:
    messageBox MB_YESNO|MB_ICONQUESTION "$(^Name) needs Visual Studio 2017 Redistributable packages.$\nDo you want to download and install them?" IDNO not_install_redist

    ${If} ${RunningX64}
        NSISdl::download https://download.visualstudio.microsoft.com/download/pr/20ef12bb-5283-41d7-90f7-eb3bb7355de7/8b58fd89f948b2430811db3da92299a6/vc_redist.x64.exe $TEMP\vc_redist_x64.exe
        Pop $R0 ; Get the return value
        StrCmp $R0 "success" 0 +3
        ExecWait "$TEMP\vc_redist_x64.exe"
        Goto +2
        MessageBox MB_OK "vc_redist_x64.exe download failed: $R0"
    ${Else}
        NSISdl::download  https://download.visualstudio.microsoft.com/download/pr/749aa419-f9e4-4578-a417-a43786af205e/d59197078cc425377be301faba7dd87a/vc_redist.x86.exe $TEMP\vc_redist_x86.exe
        Pop $R0 ; Get the return value
        StrCmp $R0 "success" 0 +3
        ExecWait "$TEMP\vc_redist_x86.exe"
        Goto +2
        MessageBox MB_OK "vc_redist_x86.exe download failed: $R0"
    ${EndIf}

    not_install_redist:

FunctionEnd
