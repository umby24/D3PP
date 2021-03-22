#CPE_Extensions = 14

;-############## Macro
Procedure List_Contains(Value.b, List mList.b())
    PushListPosition(mList())
    Result = #False
    
    ResetList(mList())
    While NextElement(mList())
        If mList() = Value
            Result = #True
            Break
        EndIf
    Wend
    
    PopListPosition(mList())
    
    ProcedureReturn Result
EndProcedure

Procedure List_PopPosition(Value.b, List mList.b()) ; Sets the list position to the given element
    Result = #False
    
    ResetList(mList())
    While NextElement(mList())
        If mList() = Value
            Result = #True
            Break
        EndIf
    Wend
    
EndProcedure
;-##############

Declare CPE_Set_Env_Colors(Type.b, Red.w, Green.w, Blue.w)

Procedure CPE_Send_ExtInfo(Client_ID, Name.s, MPPass.s, Version)
    List_Store(*Network_Client_Old, Network_Client())
    
    If Network_Client_Select(Client_ID)
        Network_Client()\Player\Login_Name = Name ;Some required internal stuff :P
        Network_Client()\Player\MPPass = MPPass
        Network_Client()\Player\Client_Version = Version
        
        SendExtInfo(Client_ID, "D3 Server " + Version, #CPE_Extensions)
        
        ;Send ExtEntry for each supported extension.
        SendExtEntry(Client_ID, "CustomBlocks", 1)
        SendExtEntry(Client_ID, "EmoteFix", 1)
        SendExtEntry(Client_ID, "HeldBlock", 1)
        SendExtEntry(Client_ID, "ClickDistance", 1)
        SendExtEntry(Client_ID, "ChangeModel", 1)
        SendExtEntry(Client_ID, "ExtPlayerList", 2)
        SendExtEntry(Client_ID, "EnvWeatherType", 1)
        SendExtEntry(Client_ID, "EnvMapAppearance", 1)
        SendExtEntry(Client_ID, "MessageTypes", 1)
        SendExtEntry(Client_ID, "BlockPermissions", 1)
        SendExtEntry(Client_ID, "EnvColors", 1)
        SendExtEntry(Client_ID, "TextHotKey", 1)
        SendExtEntry(Client_ID, "HackControl", 1)
        SendExtEntry(Client_ID, "SelectionCuboid", 1)
        SendExtEntry(Client_ID, "LongerMessages", 1)
    EndIf
    
    List_Restore(*Network_Client_Old, Network_Client()) ; Wait for the rest of the negotiation to take place...
EndProcedure

Procedure CPE_Send_Extensions(Client_ID)
    List_Store(*Network_Client_Old, Network_Client())
    
    If Network_Client_Select(Client_ID)
        If CPE_GetClientExtVersion("clickdistance") = 1
            SendClickDistance(Client_ID, System_Main\Click_Distance)
        EndIf
        
        If CPE_GetClientExtVersion("customblocks") = 1
            SendCustomBlockSupportLevel(Client_ID, 1)
        Else
            Client_Login(Client_ID, Network_Client()\Player\Login_Name, Network_Client()\Player\MPPass, Network_Client()\Player\Client_Version)
        EndIf
    EndIf
    
    List_Restore(*Network_Client_Old, Network_Client()) 
EndProcedure

Procedure.s Emote_Replace(Message.s)
    Message = ReplaceString(Message, "{:)}", Chr(1))
    Message = ReplaceString(Message, "{smile2}", Chr(2))
    Message = ReplaceString(Message, "{smile}", Chr(1))
    Message = ReplaceString(Message, "{<3}", Chr(3))
    Message = ReplaceString(Message, "{heart}", Chr(3))
    Message = ReplaceString(Message, "{diamond}",Chr(4))
    Message = ReplaceString(Message, "{club}", Chr(5))
    Message = ReplaceString(Message, "{spade}", Chr(6))
    Message = ReplaceString(Message, "{male}", Chr(11))
    Message = ReplaceString(Message, "{female}", Chr(12))
    Message = ReplaceString(Message, "{note}", Chr(13))
    Message = ReplaceString(Message, "{notes}", Chr(14))
    Message = ReplaceString(Message, "{sun}", Chr(15))
    Message = ReplaceString(Message, "{!!}", Chr(19))
    Message = ReplaceString(Message, "{P}", Chr(20))
    Message = ReplaceString(Message, "{S}", Chr(21))
    Message = ReplaceString(Message, "{*}", Chr(7))
    Message = ReplaceString(Message, "{hole}", Chr(8))
    Message = ReplaceString(Message, "{circle}", Chr(9))
    Message = ReplaceString(Message, "{-}", Chr(22))
    Message = ReplaceString(Message, "{L}", Chr(28))
    Message = ReplaceString(Message, "{house}", Chr(127))
    Message = ReplaceString(Message, "{caret}", Chr(94))
    Message = ReplaceString(Message, "{tilde}", Chr(126))
    Message = ReplaceString(Message, "{'}", Chr(180))
    Message = ReplaceString(Message, "{^}", Chr(24))
    Message = ReplaceString(Message, "{v}", Chr(25))
    Message = ReplaceString(Message, "{>}", Chr(26))
    Message = ReplaceString(Message, "{<}", Chr(27))
    Message = ReplaceString(Message, "{^^}", Chr(30))
    Message = ReplaceString(Message, "{vv}", Chr(31))
    Message = ReplaceString(Message, "{>>}", Chr(16))
    Message = ReplaceString(Message, "{<<}", Chr(17))
    Message = ReplaceString(Message, "{<>}", Chr(29))
    Message = ReplaceString(Message, "{updown}", Chr(18))
    Message = ReplaceString(Message, "{updown2}", Chr(23))
    
    ProcedureReturn Message.s
EndProcedure

Procedure CPE_GetHeldBlock(Client_ID)
    Protected *Network_Client_Old, heldBlock
    
    If Network_Client_Select(Client_ID) ; Get the NetworkClient of the person getting the packet..
        If CPE_GetClientExtVersion("heldblock") = 1
            heldBlock = Network_Client()\Player\Entity\Held_Block
        Else
            heldBlock = Network_Client()\Player\Entity\Last_Material
        EndIf
    EndIf
    
    List_Restore(*Network_Client_Old, Network_Client())
    ProcedureReturn heldBlock
EndProcedure

    
Procedure CPE_HoldThis(Client_ID, Block, CanChange)
    List_Store(*Network_Client_Old, Network_Client()) ; Store the old NetworkClient
    
    If Network_Client_Select(Client_ID) ; Get the NetworkClient of the person getting the packet..
        If CPE_GetClientExtVersion("heldblock") = 1
            If Block > 49 And CPE_GetClientExtVersion("customblocks") = 0 ; If this is a custom block, and the client doesn't support it..
                List_Restore(*Network_Client_Old, Network_Client())
                ProcedureReturn
            EndIf
            
            SendHoldThis(Client_ID, Block, CanChange)
        EndIf
    EndIf
    
    List_Restore(*Network_Client_Old, Network_Client())
EndProcedure

Procedure CPE_Selection_Cuboid_Add(Client_ID, SelectionID, Label.s, StartX.w, StartY.w, StartZ.w, EndX.w, EndY.w, EndZ.w, Red.w, Green.w, Blue.w, Opacity.w)
    If (StartX > EndX) Or (StartY > EndY) Or (StartZ > EndZ)
        ProcedureReturn 0
    EndIf
    
    List_Store(*Network_Client_Old, Network_Client())
    
    If Not Network_Client_Select(Client_ID)
        List_Restore(*Network_Client_Old, Network_Client()) 
        ProcedureReturn
    EndIf
    
    If CPE_GetClientExtVersion("selectioncuboid") = 0 Or List_Contains(SelectionID, Network_Client()\Selections()) = #True
        List_Restore(*Network_Client_Old, Network_Client()) 
        ProcedureReturn
    EndIf
    
    LastElement(Network_Client()\Selections()) ; Add to our ID list.
    AddElement(Network_Client()\Selections())
    Network_Client()\Selections() = SelectionID
    
    SendSelectionBoxAdd(Client_ID, SelectionID, Label.s, StartX.w, StartY.w, StartZ.w, EndX.w, EndY.w, EndZ.w, Red.w, Green.w, Blue.w, Opacity.w)
    
    List_Restore(*Network_Client_Old, Network_Client()) 
EndProcedure

Procedure CPE_Selection_Cuboid_Delete(Client_ID, Selection_ID)
    List_Store(*Network_Client_Old, Network_Client())
    
    PrintN("Cuboid delete called.")
    PrintN("Delete " + Str(Selection_ID) + " for " + Client_ID)
    
    If Not Network_Client_Select(Client_ID)
        PrintN("Client not found")
        List_Restore(*Network_Client_Old, Network_Client()) 
        ProcedureReturn
    EndIf
    
    If CPE_GetClientExtVersion("selectioncuboid") = 0 Or List_Contains(Selection_ID, Network_Client()\Selections()) = #False
        PrintN("Cuboid or extnesion not found.")
        List_Restore(*Network_Client_Old, Network_Client()) 
        ProcedureReturn
    EndIf
    
    SendSelectionBoxDelete(Client_ID, Selection_ID)
    
    List_PopPosition(Selection_ID, Network_Client()\Selections())
    DeleteElement(Network_Client()\Selections())
    
    List_Restore(*Network_Client_Old, Network_Client()) 
EndProcedure

Procedure.i Map_Export_Get_Size_X(Filename.s)
    Map_Size_X = 0
    Map_Size_Y = 0
    Map_Size_Z = 0
    
    *Temp_Buffer = Mem_Allocate(10, #PB_Compiler_File, #PB_Compiler_Line, "Temp")
    
    If *Temp_Buffer
        
        Temp_Buffer_Offset = 0
        
        Result = GZip_Decompress_From_File(Filename, *Temp_Buffer, 10)
        If Result = 10
            File_Version = PeekL(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 4
            Map_Size_X = PeekW(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 2
            Map_Size_Y = PeekW(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 2
            Map_Size_Z = PeekW(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 2
            
        EndIf
    EndIf
    
    Mem_Free(*Temp_Buffer)
    ProcedureReturn Map_Size_X
EndProcedure

Procedure.i Map_Export_Get_Size_Y(Filename.s)
    Map_Size_X = 0
    Map_Size_Y = 0
    Map_Size_Z = 0
    
    *Temp_Buffer = Mem_Allocate(10, #PB_Compiler_File, #PB_Compiler_Line, "Temp")
    
    If *Temp_Buffer
        
        Temp_Buffer_Offset = 0
        
        Result = GZip_Decompress_From_File(Filename, *Temp_Buffer, 10)
        If Result = 10
            File_Version = PeekL(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 4
            Map_Size_X = PeekW(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 2
            Map_Size_Y = PeekW(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 2
            Map_Size_Z = PeekW(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 2
            
        EndIf
    EndIf
    
    Mem_Free(*Temp_Buffer)
    ProcedureReturn Map_Size_Y
EndProcedure

Procedure.i Map_Export_Get_Size_Z(Filename.s)
    Map_Size_X = 0
    Map_Size_Y = 0
    Map_Size_Z = 0
    
    *Temp_Buffer = Mem_Allocate(10, #PB_Compiler_File, #PB_Compiler_Line, "Temp")
    
    If *Temp_Buffer
        
        Temp_Buffer_Offset = 0
        
        Result = GZip_Decompress_From_File(Filename, *Temp_Buffer, 10)
        If Result = 10
            File_Version = PeekL(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 4
            Map_Size_X = PeekW(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 2
            Map_Size_Y = PeekW(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 2
            Map_Size_Z = PeekW(*Temp_Buffer+Temp_Buffer_Offset) : Temp_Buffer_Offset + 2
            
        EndIf
        Mem_Free(*Temp_Buffer)
    EndIf
    
    ProcedureReturn Map_Size_Z
EndProcedure

Procedure CPE_Model_Change(Client_ID, Model.s)
    List_Store(*Network_Client_Old, Network_Client())
    
    If Not Network_Client_Select(Client_ID)
        List_Restore(*Network_Client_Old, Network_Client()) 
        ProcedureReturn
    EndIf
    
    Map_ID = Network_Client()\Player\Entity\Map_ID
    Entity_ID = Network_Client()\Player\Entity\ID_Client
    Network_Client()\Player\Entity\Model = Model
    
    ForEach Network_Client()
        If Network_Client()\Logged_In = #False Or CPE_GetClientExtVersion("changemodel") = 0 Or Network_Client()\Player\Entity\Map_ID <> Map_ID
            Continue
        EndIf
        
        SendChangeModel(Network_Client()\ID, Entity_ID, Model)
    Next
    
    SendChangeModel(Network_Client()\ID, 255, Model)
    
    List_Restore(*Network_Client_Old, Network_Client()) 
EndProcedure

Procedure CPE_Aftermap_Actions(Client_ID, *MapData.Map_Data)
    List_Store(*Network_Client_Old, Network_Client())
    
    If Not *MapData
        ProcedureReturn
    EndIf
    
    If Not Network_Client_Select(Client_ID)
        List_Restore(*Network_Client_Old, Network_Client()) 
        ProcedureReturn
    EndIf
    
    If *MapData\ColorsSet = #True And CPE_GetClientExtVersion("envcolors") = 1
        R = Red(*MapData\SkyColor)
        G = Green(*MapData\SkyColor)
        B = Blue(*MapData\SkyColor)
        CPE_Set_Env_Colors(0, R, G, B)
        
        R = Red(*MapData\CloudColor)
        G = Green(*MapData\CloudColor)
        B = Blue(*MapData\CloudColor)
        CPE_Set_Env_Colors(1, R, G, B)
        
        R = Red(*MapData\FogColor)
        G = Green(*MapData\FogColor)
        B = Blue(*MapData\FogColor)
        CPE_Set_Env_Colors(2, R, G, B)
        
        R = Red(*MapData\alight)
        G = Green(*MapData\alight)
        B = Blue(*MapData\alight)
        CPE_Set_Env_Colors(3, R, G, B)
        
        R = Red(*MapData\dlight)
        G = Green(*MapData\dlight)
        B = Blue(*MapData\dlight)
        CPE_Set_Env_Colors(4, R, G, B)
    EndIf
    
    If CPE_GetClientExtVersion("envmapappearance") = 1 And *MapData\CustomAppearance = #True
        CPE_Client_Send_Map_Appearence(Network_Client()\ID, *MapData\CustomURL, *MapData\Side_Block, *MapData\Edge_Block, *MapData\Side_level) ; Send the client the map's custom parameters.
    EndIf
    
    If CPE_GetClientExtVersion("hackcontrol") = 1
        CPE_Client_Hackcontrol_Send(Network_Client()\ID, *MapData\Flying, *MapData\NoClip, *MapData\Speeding, *MapData\SpawnControl, *MapData\ThirdPerson, *MapData\JumpHeight)
    EndIf
    
    CPEAddExtPlayer()
    ;CPE_Client_Send_Hotkeys(Network_Client()\ID)
    
    List_Restore(*Network_Client_Old, Network_Client()) 
EndProcedure

Procedure CPEAddExtPlayer()
    ;CPE ExtPlayerList time.
    tempClient = Network_Client()\ID
    loginName.s = Network_Client()\Player\Login_Name
    namePrefix.s = Network_Client()\Player\Entity\Prefix
    nameSuffix.s = Network_Client()\Player\Entity\Suffix
    mapName.s = Map_Data()\Name
    CPE = CPE_GetClientExtVersion("ExtPlayerList")
    tempID = Network_Client()\Player\NameID

    PushListPosition(Network_Client())
    ForEach Network_Client()
        ;Send the new player to everyone..
        If Network_Client()\ID <> tempClient ;If it is not the new player..
            If CPE_GetClientExtVersion("ExtPlayerList") = 2
                SendExtAddPlayerName(Network_Client()\ID, tempID, loginName, namePrefix + loginName + nameSuffix, mapName, 0)
            EndIf
            
            If CPE = 2
                Map_Select_ID(Network_Client()\Player\Map_ID)
                SendExtAddPlayerName(tempClient, Network_Client()\Player\NameID, Network_Client()\Player\Login_Name, Network_Client()\Player\Entity\Prefix + Network_Client()\Player\Login_Name + Network_Client()\Player\Entity\Suffix, Map_Data()\Name, 0)  
            EndIf
        Else
            If CPE = 2
                SendExtAddPlayerName(tempClient, tempID, loginName, namePrefix + loginName + nameSuffix, mapName, 0)
            EndIf
        EndIf
    Next
    PopListPosition(Network_Client())
EndProcedure

Procedure CPE_Set_Env_Colors(Type.b, Red.w, Green.w, Blue.w)
    SendSetEnviromentColors(Network_Client()\ID, Type, Red, Green, Blue)
EndProcedure

Procedure CPE_Set_Weather(Client_ID, Weather.b)
    List_Store(*Network_Client_Old, Network_Client())
    
    If Network_Client_Select(Client_ID)
        If CPE_GetClientExtVersion("envweathertype") = 1
            SendSetWeather(Network_Client()\ID, Weather)
        EndIf
    EndIf
    
    List_Restore(*Network_Client_Old, Network_Client())
EndProcedure

Procedure CPE_Handle_Entity()
    If Network_Client()\Logged_In = #True And CPE_GetClientExtVersion("changemodel") = 1 And Entity()\Model <> "Default"
        SendChangeModel(Network_Client()\ID, Entity()\ID_Client, Entity()\Model)
    EndIf
EndProcedure

Procedure CPE_Client_Set_Block_Permissions(Client_ID, Block_ID, CanPlace, CanDelete)
    List_Store(*Network_Client_Old, Network_Client())
    
    If Network_Client_Select(Client_ID)
        If CPE_GetClientExtVersion("blockpermissions") = #True
             SendBlockPermissions(Client_ID, Block_ID, CanPlace, CanDelete)
        EndIf
    EndIf
    
    List_Restore(*Network_Client_Old, Network_Client())
EndProcedure

Procedure CPE_Client_Send_Map_Appearence(Client_ID, URL.s, Side_Block, Edge_Block, Side_Level.w)
    List_Store(*Network_Client_Old, Network_Client())
    
    If Network_Client_Select(Client_ID)
        If CPE_GetClientExtVersion("envmapappearance") = 1
            SendEnvMapAppearance(Client_ID, URL, Side_Block, Edge_Block, Side_Level)
        EndIf
    EndIf
    
    List_Restore(*Network_Client_Old, Network_Client())
EndProcedure

Procedure CPE_Client_Send_Hotkeys(Client_ID)
    List_Store(*Network_Client_Old, Network_Client())
    
    If Network_Client_Select(Client_ID)
        If CPE_GetClientExtVersion("texthotkey") = 1
            ForEach Hotkeys()
                SendTextHotkeys(Client_ID, Hotkeys()\Label, Hotkeys()\Action, Hotkeys()\Keycode, Hotkeys()\Keymods)
            Next
        EndIf
    EndIf
    
    List_Restore(*Network_Client_Old, Network_Client())
EndProcedure

Procedure CPE_Client_Hackcontrol_Send(Client_ID, Flying, Noclip, Speeding, SpawnControl, ThirdPerson, Jumpheight.w)
    List_Store(*Network_Client_Old, Network_Client())
    
    If Network_Client_Select(Client_ID)
        If CPE_GetClientExtVersion("hackcontrol") = 1
            SendHackControl(Client_ID, Flying, Noclip, Speeding, SpawnControl, ThirdPerson, Jumpheight)
        EndIf
    EndIf
    
    List_Restore(*Network_Client_Old, Network_Client())
EndProcedure

Procedure CPE_GetClientExtVersion(*client.Network_Client, Extension.s)
    Protected Result.l = 0
    
    If *client\CPE = #False ; - Skip searching for clients that don't even support CPE.
        ProcedureReturn Result
    EndIf
    
    ResetMap(*client\Extensions())
    
    If Not FindMapElement(*client\Extensions(), LCase(Extension))
        ProcedureReturn Result
    EndIf
    
    Result = *client\Extensions(LCase(Extension))
    
    ProcedureReturn Result
EndProcedure
; IDE Options = PureBasic 5.30 (Windows - x64)
; CursorPosition = 479
; FirstLine = 301
; Folding = vH--
; EnableThread
; EnableXP
; EnableOnError
; DisableDebugger
; CompileSourceDirectory