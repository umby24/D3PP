InitNetwork()

; ########################################## Variablen ##########################################

#Network_Temp_Buffer_Size = 2000
#Network_Buffer_Size = 3000000
#Network_Packet_Size = 1400

#Network_Client_Timeout = 6000*5

Structure Network_Main
    Save_File.b                   ; Zeigt an, ob gespeichert werden soll
    File_Date_Last.l              ; Datum letzter ?nderung, bei ?nderung speichern
    Server_ID.i                   ; ID des Servers
    *Buffer_Temp                  ; Buffer f?r tempor?re Aufgaben
    Timer_Rate.l                  ; Timer f?r Down/Upload-Rate
    Upload_Rate.l                 ; Uploadrate in bytes/s
    Download_Rate.l               ; Downloadrate in bytes/s
    Upload_Rate_Counter.l         ; Upload in bytes (Z?hler wird jede Sekunde 0 gesetzt und ?bernommen)
    Download_Rate_Counter.l       ; Download in bytes (Z?hler wird jede Sekunde 0 gesetzt und ?bernommen)
EndStructure

Global Network_Main.Network_Main

; - Structure is defined in Main_Structures.pbi
; ##################################################################
; !!! Struktur mit Klienten befindet sich in Main_Structures.pbi !!!
; ##################################################################
Global NewList Network_Client.Network_Client()
;Global NewMap Network_Client.Network_Client()

Structure Network_Settings
    Port.l                        ; Port des Servers / The server's port
EndStructure

Global Network_Settings.Network_Settings

; ########################################## Ladekram ############################################

Network_Main\Buffer_Temp = Mem_Allocate(#Network_Temp_Buffer_Size, #PB_Compiler_File, #PB_Compiler_Line, "Network_Main\Buffer_Temp")

; ########################################## Declares ############################################

; ########################################## Proceduren ##########################################

Procedure Network_Save(Filename.s) ; Saves network settings
    File_ID = CreateFile(#PB_Any, Filename)
    If IsFile(File_ID)
        
        WriteStringN(File_ID, "Port = "+Str(Network_Settings\Port))
        
        Network_Main\File_Date_Last = GetFileDate(Filename, #PB_Date_Modified)
        Log_Add("Network", Lang_Get("", "File saved", Filename), 0, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
        
        CloseFile(File_ID)
    EndIf
EndProcedure

Procedure Network_Load(Filename.s) ; Loads network settings
    If OpenPreferences(Filename)
        
        Port = ReadPreferenceLong("Port", Network_Settings\Port)
        
        If Network_Settings\Port <> Port
            Network_Settings\Port = Port
            Network_Start()
        EndIf
        
        Network_Main\File_Date_Last = GetFileDate(Filename, #PB_Date_Modified)
        Log_Add("Network", Lang_Get("", "File loaded", Filename), 0, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
        
        ClosePreferences()
        
    EndIf
EndProcedure

Procedure Network_Start() ; - Starts listening for connections
    If Network_Main\Server_ID
        Network_Stop()
    EndIf
    
    Network_Main\Server_ID = CreateNetworkServer(#PB_Any, Network_Settings\Port, #PB_Network_TCP)
    
    If Network_Main\Server_ID = 0
        Log_Add("Network", Lang_Get("", "Can't start server"), 10, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
    Else
        Log_Add("Network", Lang_Get("", "Server started", Str(Network_Settings\Port)), 0, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
    EndIf
EndProcedure

Procedure Network_Stop() ; - Disconnects all clients and stops listening for new connections.
    CloseNetworkServer(Network_Main\Server_ID)
    Network_Main\Server_ID = 0
    
    ForEach Network_Client()
        Network_Client_Delete(Network_Client()\ID, Lang_Get("", "Disconnected"), 1)
    Next
    
    Log_Add("Network", Lang_Get("", "Server stopped"), 0, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
EndProcedure

Procedure Network_HTML_Stats() ; - Generates the network stats HTML page
    Generation_Time = Milliseconds()
    
    File_ID = CreateFile(#PB_Any, Files_File_Get("Network_HTML"))
    If IsFile(File_ID)
        
        WriteStringN(File_ID, "<html>")
        WriteStringN(File_ID, "  <head>")
        WriteStringN(File_ID, "    <title>Minecraft-Server Network</title>")
        WriteStringN(File_ID, "  </head>")
        WriteStringN(File_ID, "  <body>")
        
        WriteStringN(File_ID, "      <b><u>Overview:</u></b><br>")
        WriteStringN(File_ID, "      Port: "+Str(Network_Settings\Port)+".<br>")
        WriteStringN(File_ID, "      Download_Rate: "+StrD(Network_Main\Download_Rate/1024, 3)+"kbytes/s.<br>")
        WriteStringN(File_ID, "      Download_Rate: <font color="+Chr(34)+"#FF0000"+Chr(34)+">"+LSet("", Network_Main\Download_Rate*10/1024, "|")+"</font><br>")
        WriteStringN(File_ID, "      Upload_Rate: "+StrD(Network_Main\Upload_Rate/1024, 3)+"kbytes/s.<br>")
        WriteStringN(File_ID, "      Upload_Rate: <font color="+Chr(34)+"#FF0000"+Chr(34)+">"+LSet("", Network_Main\Upload_Rate*10/1024, "|")+"</font><br>")
        
        WriteStringN(File_ID, "      <br>")
        WriteStringN(File_ID, "      <br>")
        WriteStringN(File_ID, "      <br>")
        
        WriteStringN(File_ID, "      <b><u>Clients:</u></b><br>")
        WriteStringN(File_ID, "      <br>")
        WriteString(File_ID,  "      <table border=1>")
        WriteStringN(File_ID, "        <tr>")
        WriteStringN(File_ID, "          <th><b>ID</b></th>")
        WriteStringN(File_ID, "          <th><b>Login_Name</b></th>")
        WriteStringN(File_ID, "          <th><b>Client_Version</b></th>")
        WriteStringN(File_ID, "          <th><b>IP</b></th>")
        WriteStringN(File_ID, "          <th><b>Download_Rate</b></th>")
        WriteStringN(File_ID, "          <th><b>Upload_Rate</b></th>")
        WriteStringN(File_ID, "          <th><b>Entity_ID</b></th>")
        WriteStringN(File_ID, "        </tr>")
        ForEach Network_Client()
            WriteStringN(File_ID, "        <tr>")
            WriteStringN(File_ID, "          <td>"+Str(Network_Client()\ID)+"</td>")
            WriteStringN(File_ID, "          <td>"+Network_Client()\Player\Login_Name+"</td>")
            WriteStringN(File_ID, "          <td>"+Str(Network_Client()\Player\Client_Version)+"</td>")
            WriteStringN(File_ID, "          <td>"+Network_Client()\IP+"</td>")
            WriteStringN(File_ID, "          <td>"+StrD(Network_Client()\Download_Rate/1000, 3)+"kB/s</td>")
            WriteStringN(File_ID, "          <td>"+StrD(Network_Client()\Upload_Rate/1000, 3)+"kB/s</td>")
            If Network_Client()\Player\Entity
                WriteStringN(File_ID, "          <td>"+Str(Network_Client()\Player\Entity\ID)+"</td>")
            EndIf
            WriteStringN(File_ID, "        </tr>")
        Next
        WriteString(File_ID,  "      </table>")
        
        WriteStringN(File_ID, "      <br>")
        WriteStringN(File_ID, "      <br>")
        WriteStringN(File_ID, "      <br>")
        
        WriteStringN(File_ID, "      Site generated in "+Str(Milliseconds()-Generation_Time)+" ms. "+FormatDate("%hh:%ii:%ss  %dd.%mm.%yyyy", Date())+" ("+Str(Date())+")<br>")
        
        WriteStringN(File_ID, "  </body>")
        WriteStringN(File_ID, "</html>")
        
        CloseFile(File_ID)
    EndIf
EndProcedure

Procedure Network_Client_Count() ; - Gets the size of network_client() list (Number of connected clients, verified or not)
    ProcedureReturn ListSize(Network_Client())
EndProcedure

Procedure Network_Client_Get_Pointer(Client_ID, Log=1)    ; Gets a pointer to the given linked-list object
    If ListIndex(Network_Client()) <> -1 And Network_Client()\ID = Client_ID
        ProcedureReturn Network_Client()
    Else
        ForEach Network_Client()
            If Network_Client()\ID = Client_ID
                ProcedureReturn Network_Client()
            EndIf
        Next
    EndIf
    
    If Log
        Log_Add("Network", Lang_Get("", "Can't find Network_Client()\ID = [Field_0]", Str(Client_ID)), 5, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
    EndIf
    ProcedureReturn 0
EndProcedure

Procedure Network_Client_Select(Client_ID, Log=1)    ; Selects the linked-list object
    If ListIndex(Network_Client()) <> -1 And Network_Client()\ID = Client_ID
        ProcedureReturn #True
    Else
        ForEach Network_Client()
            If Network_Client()\ID = Client_ID
                ProcedureReturn #True
            EndIf
        Next
    EndIf
    
    If Log
        Log_Add("Network", Lang_Get("", "Can't find Network_Client()\ID = [Field_0]", Str(Client_ID)), 5, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
    EndIf
    ProcedureReturn #False
EndProcedure

Procedure Network_Client_Add(Client_ID)     ; Fügt einen Clienten hinzu / Adds a client
    If ListIndex(Network_Client()) <> -1
        *Network_Client_Old = Network_Client()
    Else
        *Network_Client_Old = 0
    EndIf
    
    AddElement(Network_Client())
    Network_Client()\ID = Client_ID
    Network_Client()\IP = IPString(GetClientIP(Client_ID))
    Network_Client()\Buffer_Input = Mem_Allocate(#Network_Buffer_Size, #PB_Compiler_File, #PB_Compiler_Line, "Network_Client("+Str(Client_ID)+")\Buffer_Input")
    Network_Client()\Buffer_Input_Offset = 0
    Network_Client()\Buffer_Input_Available = 0
    Network_Client()\Buffer_Output = Mem_Allocate(#Network_Buffer_Size, #PB_Compiler_File, #PB_Compiler_Line, "Network_Client("+Str(Client_ID)+")\Buffer_Output")
    Network_Client()\Buffer_Output_Offset = 0
    Network_Client()\Buffer_Output_Available = 0
    Network_Client()\Last_Time_Event = Milliseconds()
    
    If Network_Client()\Buffer_Input = 0
        Log_Add("Network", Lang_Get("", "Can't allocate receive-memory", Str(Client_ID)), 10, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
    EndIf
    
    If Network_Client()\Buffer_Output = 0
        Log_Add("Network", Lang_Get("", "Can't allocate send-memory", Str(Client_ID)), 10, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
    EndIf
    
    If Network_Client()\Buffer_Input = 0 Or Network_Client()\Buffer_Output = 0
        If Network_Client()\Buffer_Input : Mem_Free(Network_Client()\Buffer_Input) : EndIf
        If Network_Client()\Buffer_Output : Mem_Free(Network_Client()\Buffer_Output) : EndIf
        Network_Client_Kick(Client_ID, Lang_Get("", "Redscreen: Can't allocate memory"), 1)
        DeleteElement(Network_Client())
    Else
        Log_Add("Network", Lang_Get("", "Client created", Str(Network_Client()\ID), Network_Client()\IP), 0, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
        Plugin_Event_Client_Add(Network_Client())
    EndIf
    
    If *Network_Client_Old
        ChangeCurrentElement(Network_Client(), *Network_Client_Old)
    EndIf
EndProcedure

Procedure Network_Client_Delete(Client_ID, Message.s, Show_2_All)     ; Deletes a client
    If Network_Client_Select(Client_ID)
        
        Plugin_Event_Client_Delete(Network_Client())
        
        Client_Logout(Client_ID, Message, Show_2_All)
        Mem_Free(Network_Client()\Buffer_Input)
        Mem_Free(Network_Client()\Buffer_Output)
        Log_Add("Network", Lang_Get("", "Client deleted", Str(Network_Client()\ID), Network_Client()\IP, Message), 0, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
        DeleteElement(Network_Client())
    EndIf
EndProcedure

Procedure Network_Client_Kick(*Client.Network_Client, Message.s, Hide) ; Kick the client
    System_Red_Screen(*Client\ID, Message) ; - Sends the kick packet
    
    If *Client\Disconnect_Time = 0
        *Client\Disconnect_Time = Milliseconds() + 1000
        *Client\Logged_In = 0
        *Client\Player\Logout_Hide = Hide
        Log_Add("Network_Client", Lang_Get("", "Client kicked", *Client\Player\Login_Name, Message), 0, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
    EndIf
EndProcedure

Procedure Network_Client_Ping(Client_ID) ; Send a ping to the specified client.
    List_Store(*Network_Client_Old, Network_Client())
    
    If Network_Client_Select(Client_ID)
        Network_Client_Output_Write_Byte(Client_ID, 1)
    EndIf
    
    List_Restore(*Network_Client_Old, Network_Client())
EndProcedure

Procedure Network_Input_Do()  ; Wertet die empfangenen Daten aus. / Evaluates received data
    ForEach Network_Client()
        Repeat_Max = 10 ; Anzahl maximaler Durchläufe der Schleife für jeden client / Maximum number of iterations of the loop for each client
        
        List_Store(*Network_Client_Old, Network_Client())
        
        While InputBufferAvailable(Network_Client()) >= 1 And Repeat_Max > 0
            Command_Byte = (ClientInputReadByte(Network_Client())& 255)
            InputAddOffset(Network_Client(), -1)
            
            Network_Client()\Last_Time_Event = Milliseconds()
            
            
            Select Command_Byte
                Case 0 ; ################ Login
                    If InputBufferAvailable(Network_Client()) >= 1 + 1 + 64 + 64 + 1
                        HandleHandshake(Network_Client())
                    EndIf
                    
                Case 1 ; ############### Ping
                    If InputBufferAvailable(Network_Client()) >= 1
                        HandlePing(Network_Client())
                    EndIf
                    
                Case 5 ; ############### Blockänderung / Block Change"
                    If InputBufferAvailable(Network_Client()) >= 9
                        HandleBlockChange(Network_Client())
                    EndIf
                    
                Case 8 ; ############### Spielerbewegung / Player Movement
                    If InputBufferAvailable(Network_Client()) >= 10
                        HandlePlayerTeleport(Network_Client())
                    EndIf
                    
                Case 13 ; ############### Nachricht kommt herein / Chat message
                    If InputBufferAvailable(Network_Client()) >= 66
                        HandleChatPacket(Network_Client())
                    EndIf
                    
                Case 16 ; CPE ExtInfo Packet
                    If InputBufferAvailable(Network_Client()) >= 67
                        HandleExtInfo(Network_Client())
                    EndIf
                    
                Case 17 ; CPE ExtEntry Packet
                    If InputBufferAvailable(Network_Client()) >= 1 + 64 + 4
                        HandleExtEntry(Network_Client())
                    EndIf
                    
                Case 19 ; CPE Custom Block Support Level
                    If InputBufferAvailable(Network_Client()) >= 2
                        HandleCustomBlockSupportLevel(Network_Client())
                    EndIf
                    
                Default ; Wenn Befehl nicht gefunden / When packet isn't found
                    Log_Add("Network", Lang_Get("", "Unknown data", Str(Network_Client()\ID), Str(Command_Byte)), 5, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
                    Network_Client_Kick(Network_Client()\ID, Lang_Get("", "Networkerror"), 0)
                    
            EndSelect
            
            Repeat_Max - 1
        Wend
        
        List_Restore(*Network_Client_Old, Network_Client())
        
    Next
EndProcedure

Procedure Network_Output_Do()  ; Client timeout checking (Sends pings)
    ForEach Network_Client()
        If Network_Client()\Ping_Time < Milliseconds()
            Network_Client()\Ping_Time = Milliseconds() + 5000
            Network_Client()\Ping_Sent_Time  = Milliseconds()
            Network_Client_Ping(Network_Client()\ID)
        EndIf
    Next
EndProcedure

Procedure Network_Output_Send() ; Sends data from the send buffers
    ; Kleinere Datenmengen zuerst senden. Verhindert Lag durch Senden der Karte - Below line sends to clients with smaller output buffers, to prevent lag caused during map sends.
    ;SortStructuredList(Network_Client(), #PB_Sort_Ascending, OffsetOf(Network_Client\Buffer_Output_Available), #PB_Integer)
    
    ForEach Network_Client()
        While Network_Client_Output_Available(Network_Client())
            Data_Size = Network_Client_Output_Available(Network_Client())
            If Data_Size > #Network_Packet_Size : Data_Size = #Network_Packet_Size : EndIf ; - Breaks packets into smaller chunks if they exceed a certain size
            If Data_Size > #Network_Temp_Buffer_Size : Data_Size = #Network_Temp_Buffer_Size : EndIf 
            
            Network_Client_Output_Read_Buffer(Network_Client(), Network_Main\Buffer_Temp, Data_Size) ; - Pulls data from the output buffer
            Network_Client_Output_Add_Offset(Network_Client(), -Data_Size) ; - ???
            
            Bytes_Sent = SendNetworkData(Network_Client(), Network_Main\Buffer_Temp, Data_Size) ; - Sends the data across the network.
            
            If Bytes_Sent > 0
                Network_Client_Output_Add_Offset(Network_Client(), Bytes_Sent) ; - Pushes the offset of the send buffer up so the sent data wont be sent again
                Network_Client()\Upload_Rate_Counter + Bytes_Sent ; - Updates the upload counter for this user/
                Network_Main\Upload_Rate_Counter + Bytes_Sent
            ElseIf Bytes_Sent = 0
                Log_Add("Network", Lang_Get("", "Can't send data", Str(Network_Client()\ID), Str(Bytes_Sent)), 10, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
                Network_Client_Kick(Network_Client(), Lang_Get("", "Networkerror"), 0)
                Break
            ElseIf Bytes_Sent = -1
                ; - Translation: Failed to send to the client, Function result: [Field_1]
                ;Log_Add("Network", Lang_Get("", "Fehler beim Senden beim Klient [Field_0]. Ergebnis der Funktion: [Field_1]", Str(Network_Client()\ID), Str(Bytes_Sent)), 10, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
                Break
            EndIf
        Wend
    Next
EndProcedure

Procedure Network_Events()
    
    Watchdog_Watch("Network", "Begin Events", 0)
    
    Repeat
        Server_Event = NetworkServerEvent()
        Select Server_Event
                
            Case #PB_NetworkEvent_None
                Break
                
            Case #PB_NetworkEvent_Connect
                Client_ID = EventClient()
                If Client_ID
                    If Network_Client_Select(Client_ID, 0) = #False
                        Network_Client_Add(Client_ID)
                    Else
                        Log_Add("Network", Lang_Get("", "Network_Client()\ID exists already", Str(Client_ID)), 10, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
                    EndIf
                Else
                    Log_Add("Network", Lang_Get("", "Network_Client()\ID = 0"), 10, #PB_Compiler_File, #PB_Compiler_Line, #PB_Compiler_Procedure)
                EndIf
                
            Case #PB_NetworkEvent_Data
                Client_ID = EventClient()
                Data_Size = ReceiveNetworkData(Client_ID, Network_Main\Buffer_Temp, #Network_Temp_Buffer_Size) ; - Read data off the socket
                If Data_Size > 0
                    If Network_Client_Select(Client_ID)
                        InputWriteBuffer(Network_Client(), Network_Main\Buffer_Temp, Data_Size) ; - Write the data to the client input buffer to be processed elsewhere.
                        Network_Client()\Download_Rate_Counter + Data_Size
                    EndIf
                    
                    Network_Main\Download_Rate_Counter + Data_Size
                    ;WriteData(tempfile, *Temp_Buffer, Data_Size)
                ElseIf Data_Size = -1
                    Network_Client_Delete(Client_ID, Lang_Get("", "Networkerror"), 1)
                EndIf
                
            Case #PB_NetworkEvent_Disconnect
                Client_ID = EventClient()
                Network_Client_Delete(Client_ID, Lang_Get("", "Disconnected"), 1)
                
        EndSelect
    ForEver
    
    ForEach Network_Client()
        Client_ID = Network_Client()\ID
        If Network_Client()\Disconnect_Time > 0 And Network_Client()\Disconnect_Time < Milliseconds()
            Network_Client_Delete(Client_ID, Lang_Get("", "Forced disconnect"), 1)
            CloseNetworkConnection(Client_ID)
        ElseIf Network_Client()\Last_Time_Event + #Network_Client_Timeout < Milliseconds()
            Network_Client_Delete(Client_ID, Lang_Get("", "Timeout"), 1)
            CloseNetworkConnection(Client_ID)
        EndIf
    Next
    
    Watchdog_Watch("Network", "End Events", 2)
    
EndProcedure

Procedure Network_Main()
    Watchdog_Watch("Main", "Before: Network_Main()", 1)
    
    If Network_Main\Save_File
        Network_Main\Save_File = 0
        Network_Save(Files_File_Get("Network"))
    EndIf
    
    File_Date = GetFileDate(Files_File_Get("Network"), #PB_Date_Modified)
    
    If Network_Main\File_Date_Last <> File_Date
        Network_Load(Files_File_Get("Network"))
    EndIf
EndProcedure

Procedure UpdateNetworkStats()
    ForEach Network_Client()
        Network_Client()\Download_Rate = Network_Client()\Download_Rate_Counter / 5
        Network_Client()\Upload_Rate = Network_Client()\Upload_Rate_Counter / 5
        Network_Client()\Download_Rate_Counter = 0
        Network_Client()\Upload_Rate_Counter = 0
    Next
    
    Network_Main\Download_Rate = Network_Main\Download_Rate_Counter / 5
    Network_Main\Upload_Rate = Network_Main\Upload_Rate_Counter / 5
    Network_Main\Download_Rate_Counter = 0
    Network_Main\Upload_Rate_Counter = 0
    
    Network_HTML_Stats()
EndProcedure

RegisterCore("Network_Main", 1000, #Null, @Network_Stop(), @Network_Main())
RegisterCore("Network_Stats", 5000, #Null, #Null, @UpdateNetworkStats())
RegisterCore("Network_Events", 1, #Null, #Null, @Network_Events())
RegisterCore("Network_Output_Send", 0, #Null, #Null, @Network_Output_Send())
RegisterCore("Network_Output_Do", 0, #Null, #Null, @Network_Output_Do())
RegisterCore("Network_Input_Do", 0, #Null, #Null, @Network_Input_Do())
; IDE Options = PureBasic 5.30 (Windows - x64)
; CursorPosition = 29
; Folding = gAA-
; EnableUnicode
; EnableXP