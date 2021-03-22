; ####################################### System

Procedure System_Login_Screen(Client_ID, Message_0.s, Message_1.s, Op_Mode)
    Protected Text_0.s, Text_1.s, Blank.s
    Message_0 = String_GV(Message_0)
    Message_1 = String_GV(Message_1)
    Text_0.s = LSet(Message_0, 64, " ")
    Text_1.s = LSet(Message_1, 64, " ")
    Blank.s = LSet("", 64, " ")
    
    If Text_0 <> Blank And Text_1 <> Blank
        SendClientHandshake(Client_ID, 7, Text_0, Text_1, Op_Mode)
    EndIf
EndProcedure

Procedure System_Red_Screen(Client_ID, Message.s)
    Protected Text.s
    Message = String_GV(Message)
    Text.s = LSet(Message, 64, " ")
    
    If Text <> LSet("", 64, " ")
        SendDisconnect(Client_ID, Text)
    EndIf
EndProcedure

Procedure System_Message_Network_Send(Client_ID, Message.s, Type=0) ; Sendet eine Nachricht zu einem Clienten
    Protected Lines.l, Text.s, *client.Network_Client
    *client = Network_Client_Get_Pointer(Client_ID)
    
    Message = ReplaceString(Message, "<br>", Chr(10))
    
    Message = String_Multiline(Message)
    
    Message = String_GV(Message)
    
    Message = Emote_Replace(Message)
    
    Lines = CountString(Message, Chr(10)) + 1
    
    For i = 1 To Lines
        Text.s = StringField(Message, i, Chr(10))
        
        If String_IV(Right(Text, 1)) And CPE_GetClientExtVersion(*client, "emotefix") = 0
            Text + "." ; Suffix emotes for non-fixed clients.
        EndIf
        
        Text = LSet(Text, 64, " ")
        
        If Text <> LSet("", 64, " ") And Text <> ""
            SendChatMessage(Client_ID, Text, Type)
        EndIf
    Next
EndProcedure

Procedure System_Message_Network_Send_2_All(Map_ID, Message.s, Type=0) ; Sendet eine Nachricht zu allen Clienten
    If ListIndex(Network_Client()) <> -1
        *Network_Client_Old = Network_Client()
    Else
        *Network_Client_Old = 0
    EndIf
    
    Message = ReplaceString(Message, "<br>", Chr(10))
    
    Message = String_Multiline(Message)
    
    Message = String_GV(Message)
    
    Message = Emote_Replace(Message)
    
    Lines = CountString(Message, Chr(10)) + 1
    
    For i = 1 To Lines
        Text.s = StringField(Message, i, Chr(10))
        Text = LSet(Text, 64, " ")
        
        If Text <> LSet("", 64, " ") And Text <> ""
            ForEach Network_Client()
                If Network_Client()\Player\Map_ID = Map_ID Or Map_ID = -1
                    If String_IV(Right(Text, 1)) And CPE_GetClientExtVersion(Network_Client(), "emotefix") = 0
                        Text = Text + "." ; Suffix emotes for non-fixed clients.
                    EndIf
                    
                    SendChatMessage(Network_Client()\ID, Text, Type)
                EndIf
            Next
        EndIf
    Next
    
    
    If *Network_Client_Old
        ChangeCurrentElement(Network_Client(), *Network_Client_Old)
    EndIf
EndProcedure

; ######################################## Map

Procedure Network_Out_Block_Set(Client_ID, X, Y, Z, Type.a) ; Sendet einen Block zu "Client_ID"
    If Network_Client_Select(Client_ID) And Network_Client()\Logged_In
        If Block(Type)\CPE_Level > Network_Client()\CustomBlocks_Level
            Type = Block(Type)\CPE_Replace
        Else
            Type = Block(Type)\On_Client
        EndIf
        
        SendBlockChange(Client_ID, X, Y, Z, Type)
    EndIf
EndProcedure

Procedure Network_Out_Block_Set_2_Map(Map_ID, X, Y, Z, Type.a) ; Sendet einen Block zu allen Clienten einer Karte
    ForEach Network_Client()
        If Network_Client()\Player\Map_ID = Map_ID And Network_Client()\Logged_In
            If Block(Type)\CPE_Level > Network_Client()\CustomBlocks_Level
                Type = Block(Type)\CPE_Replace
            Else
                Type = Block(Type)\On_Client
            EndIf
            
            SendBlockChange(Network_Client()\ID, X, Y, Z, Type)
        EndIf
    Next
EndProcedure

; ######################################### Player

Procedure Network_Out_Entity_Add(Client_ID, ID_Client, Name.s, X.f, Y.f, Z.f, Rotation.f, Look.f) ; Erstellt ein neues Spielerobjekt auf einem Client
    
    If ListIndex(Network_Client()) <> -1
        *Network_Client_Old = Network_Client()
    Else
        *Network_Client_Old = 0
    EndIf
    
    If Network_Client_Select(Client_ID)
        Name.s = LSet(Name, 64, " ")
        X = X*32
        Y = Y*32
        Z = Z*32 + 51
        Rotation = Rotation/360*256
        Look = Look/360*256
        
        If CPE_GetClientExtVersion("ExtPlayerList") < 2
            SendSpawnEntity(Client_ID, ID_Client, Name, X, Y, Z, Rotation, Look)
        Else
            SendExtAddEntity2(Client_ID, ID_Client, Name, Name, X, Y, Z, Rotation, Look) ; TODO: Provide the clients login name..
        EndIf
        
        ;     If Network_Client()\ExtPlayerList = #False
        ;     Else
        ;       Network_Client_Output_Write_Byte(Client_ID, 23) ; CPE ExtAddEntity
        ;       Network_Client_Output_Write_Byte(Client_ID, ID_Client)
        ;       Network_Client_Output_Write_String(Client_ID, Name, 64)
        ;       Network_Client_Output_Write_String(Client_ID, Name, 64)
        ;       System_Message_Network_Send_2_All(-1, "Sent ExtAddEntity")
        ;     EndIf
    Else
        PrintN("Could not select client")
    EndIf
    
    If *Network_Client_Old
        ChangeCurrentElement(Network_Client(), *Network_Client_Old)
    EndIf
EndProcedure

Procedure Network_Out_Entity_Delete(Client_ID, ID_Client) ; Löscht eine Spielerobjekt auf einem Client
    SendDespawnEntity(Client_ID, ID_Client)
EndProcedure

Procedure Network_Out_Entity_Position(Client_ID, ID_Client, X.f, Y.f, Z.f, Rotation.f, Look.f) ; Sendet die Spielerbewegung von "Client_ID" zu allen
    X = X*32
    Y = Y*32
    Z = Z*32 + 51
    Rotation = Rotation/360*256
    Look = Look/360*256
    
    SendPlayerTeleport(Client_ID, ID_Client, X, Y, Z, Rotation, Look)
EndProcedure
; IDE Options = PureBasic 5.30 (Windows - x64)
; CursorPosition = 85
; FirstLine = 24
; Folding = --
; EnableXP
; DisableDebugger
; CompileSourceDirectory