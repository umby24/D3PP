Declare UnregisterCore(Name.s)

Declare RegisterCore(Name.s, Timer.i, *InitFunction, *ShutdownFunction, *MainFunction)

Declare CoreLoop()

Declare CoreInit()

Declare CoreShutdown()

Declare Files_Save(Filename.s)

Declare Files_Load(Filename.s)

Declare.s Files_File_Get(Name.s)

Declare.s Files_Folder_Get(Name.s)

Declare Mem_HTML_Stats()

Declare Mem_Allocate(Size, File.s, Line, Message.s)

Declare Mem_Free(*Memory)

Declare Mem_Get_WorkingSetSize()

Declare Mem_Get_PagefileUsage()

Declare Mem_Main()

Declare Watchdog_Thread(*Dummy)

Declare Watchdog_Thread_ID_Set(wModule.s, Thread_ID)

Declare Watchdog_Main()

Declare Log_File_Size_Check() ; Checks the date and size of the log, and rotates if nessicary.

Declare Log_File_Write(Filename.s) ; Writes the last element in the log (by date)

Declare Log_Load()

Declare Log_Add(wModule.s, Message.s, Type, PB_File.s, PB_Line, PB_Procedure.s) ; Saves a log entry

Declare.s String_GV(Input.s) ; Validiert den String / Validates string (replaces invalid characters)

Declare String_IV(Input.s) ; Prüft ob der String valid ist

Declare.s String_Multiline(Input.s) ; Teilt einen String in mehrere Zeilen auf / Split a string into multiple lines

Declare String_Main()

Declare Language_Strings_Load(Filename.s)

Declare Language_Strings_Save(Filename.s)

Declare.s Lang_Get(Language.s, Input.s, Field_0.s = "", Field_1.s = "", Field_2.s = "", Field_3.s = "") ; Wandelt einen String zur passenden Sprache  

Declare Language_Main()

Declare Network_Save(Filename.s) ; Saves network settings

Declare Network_Load(Filename.s) ; Loads network settings

Declare Network_Start() ; - Starts listening for connections

Declare Network_Stop() ; - Disconnects all clients and stops listening for new connections.

Declare Network_HTML_Stats() ; - Generates the network stats HTML page

Declare Network_Client_Count() ; - Gets the size of network_client() list (Number of connected clients, verified or not)

Declare Network_Client_Get_Pointer(Client_ID, Log=1)    ; Gets a pointer to the given linked-list object

Declare Network_Client_Select(Client_ID, Log=1)    ; Selects the linked-list object

Declare Network_Client_Add(Client_ID)     ; Fügt einen Clienten hinzu / Adds a client

Declare Network_Client_Delete(Client_ID, Message.s, Show_2_All)     ; Deletes a client

Declare Network_Client_Kick(Client_ID, Message.s, Hide) ; Kick the client

Declare Network_Client_Ping(Client_ID) ; Send a ping to the specified client.

Declare Network_Input_Do()  ; Wertet die empfangenen Daten aus. / Evaluates received data

Declare Network_Output_Do()  ; Client timeout checking (Sends pings)

Declare Network_Output_Send() ; Sends data from the send buffers

Declare Network_Events()

Declare Network_Main()

Declare UpdateNetworkStats()

Declare Network_Client_Output_Available(Client_ID)     ; Returns the number of bytes in the output buffer.

Declare Network_Client_Output_Add_Offset(Client_ID, Bytes)     ; Addiert einige Bytes zum Offset des Sendebuffers

Declare Network_Client_Output_Read_Buffer(Client_ID, *Data_Buffer, Data_Size)   ; Liest Daten aus dem Sendebuffer

Declare Network_Client_Output_Write_Byte(Client_ID, Value.b)     ; Writes a byte to the send buffer

Declare.w EndianW(val.w) ; Change Endianness of a Short (Word). Yay inline ASM!

Declare.l Endian(val.l) ; Change endianness of an int (long, DWORD, etc)

Declare Network_Client_Output_Write_Word(Client_ID, Value.w)     ; Write a short to the send buffer

Declare Network_Client_Output_Write_Int(Client_ID, Value.l) ;Using 'Long' here, because in this context, it is an int (4 Bytes)

Declare Network_Client_Output_Write_String(Client_ID, String.s, Length)     ; Write a string of the given length to the sendbuffer

Declare Network_Client_Output_Write_Buffer(Client_ID, *Data_Buffer, Data_Size)     ; Write raw bytes into the send buffer.

Declare InputBufferAvailable(*Client.Network_Client)     ;  -- Bytes available in the receive buffer

Declare InputAddOffset(*Client.Network_Client, Bytes)     ; Addiert einige Bytes zum Offset des Empfangbuffers -- Adds some bytes to offset the receive buffer

Declare.b ClientInputReadByte(*Client.Network_Client)     ; Liest ein Byte aus dem Empfangsbuffer -- Reads a byte from the receive buffer

Declare.w ClientInputReadShort(*Client.Network_Client)     ; Liest ein Byte aus dem Empfangsbuffer -- Reads a short from the receive buffer

Declare.l ClientInputReadInt(*Client.Network_Client)

Declare.s ClientInputReadString(*Client.Network_Client, Length)     ; Liest ein String angegebener L?nge aus dem Empfangsbuffer -- Reads a string of specified length from the receive buffer

Declare ClientInputReadBytes(*Client.Network_Client, *Data_Buffer, Data_Size)   ; Liest Daten aus dem Empfangsbuffer -- Reads data from the receive buffer

Declare InputWriteBuffer(*Client.Network_Client, *Data_Buffer, Data_Size)   ; Schreibt Daten in den Empfangsbuffer -- Write data in the receive buffer

Declare HandleHandshake(*Client.Network_Client)

Declare HandlePing(*Client.Network_Client)

Declare HandleBlockChange(*Client.Network_Client)

Declare HandlePlayerTeleport(*Client.Network_Client)

Declare HandleChatPacket(*Client.Network_Client)

Declare HandleExtInfo(*Client.Network_Client)

Declare HandleExtEntry(*Client.Network_Client)

Declare HandleCustomBlockSupportLevel(*Client.Network_Client)

Declare SendExtInfo(ClientID, Server.s, Extensions.w)

Declare SendExtEntry(ClientID, ExtName.s, ExtVersion.l)

Declare SendClickDistance(ClientID, Distance.w)

Declare SendCustomBlockSupportLevel(ClientID, SupportLevel.b)

Declare SendHoldThis(ClientID, HeldBlock.b, PreventChange.b)

Declare SendTextHotkeys(ClientID, Label.s, Action.s, Keycode.l, Keymod.b)

Declare SendExtAddPlayerName(ClientID, NameId.w, Playername.s, Listname.s, Groupname.s, Grouprank.b)

Declare SendExtRemovePlayerName(ClientID, NameID.w)

Declare SendSetEnviromentColors(ClientID, Type.b, Red.w, Green.w, Blue.w)

Declare SendSelectionBoxAdd(ClientID, SelectionID, Label.s, StartX.w, StartY.w, StartZ.w, EndX.w, EndY.w, EndZ.w, Red.w, Green.w, Blue.w, Opacity.w)

Declare SendSelectionBoxDelete(ClientID, SelectionId)

Declare SendBlockPermissions(ClientID, BlockId.b, CanPlace.b, CanDelete.b)

Declare SendChangeModel(ClientID, EntityId, Model.s)

Declare SendEnvMapAppearance(ClientID, Url.s, Sideblock.b, Edgeblock.b, Sidelevel.w)

Declare SendSetWeather(ClientID, Weather.b)

Declare SendHackControl(ClientID, Flying.b, Noclip.b, Speeding.b, SpawnControl.b, ThirdPerson.b, Jumpheight.w)

Declare SendExtAddEntity2(ClientID, EntityId.b, IGN.s, Skin.s, X.w, Y.w, Z.w, Rotation.b, Look.b)

Declare SendClientHandshake(ClientID, ProtocolVersion.b, ServerName.s, ServerMotd.s, UserType.b)

Declare SendMapInit(ClientID)

Declare SendMapData(ClientID, chunkSize.w, *Data, PercentComplete.a)

Declare SendMapFinalize(ClientID, X.w, Y.w, Z.w)

Declare SendBlockChange(ClientID, X.w, Y.w, Z.w, Type.a)

Declare SendSpawnEntity(ClientID, PlayerId.b, Name.s, X.w, Y.w, Z.w, Rotation.b, Look.b)

Declare SendPlayerTeleport(ClientID, PlayerId.b, X.w, Y.w, Z.w, Rotation.b, Look.b)

Declare SendDespawnEntity(ClientID, PlayerId.b)

Declare SendChatMessage(ClientID, Message.s, Location.b)

Declare SendDisconnect(ClientID, Reason.s)

Declare Error_Handler()

Declare Error_Main()

Declare System_Save(Filename.s) ; Speichert die Einstellungen

Declare System_Load(Filename.s) ; Lädt die Einstellungen

Declare System_Main()

Declare Block_Load(Filename.s) ; - Loads the Blocks.txt file, which contains all blocks the server recognizes and its respective settings.

Declare Block_Save(Filename.s) ; - Saves the blocks and all block data.

Declare Block_Get_Pointer(Number) ; Specifices a pointer back to the element.

Declare Block_Main()

Declare BlockShutdown()

Declare Location_Load(Filename.s)

Declare Location_Save(Filename.s)

Declare Location_Select(Name.s) ; Wählt das Linked-List-Objekt

Declare Location_Add(Name.s, Map_ID, X.f, Y.f, Z.f, Rot.f, Look.f)

Declare Location_Delete(Name.s)

Declare Location_Main()

Declare Teleporter_Select(*Map_Data.Map_Data, ID.s) ; Wählt das Linked-List-Objekt

Declare Teleporter_Get_Pointer(*Map_Data.Map_Data, ID.s)

Declare Teleporter_Add(*Map_Data.Map_Data, ID.s, X_0, Y_0, Z_0, X_1, Y_1, Z_1, Dest_Map_Unique_ID.s, Dest_Map_ID, X.f, Y.f, Z.f, Rot.f, Look.f) 

Declare Teleporter_Delete(*Map_Data.Map_Data, ID.s)

Declare Rank_Load(Filename.s)

Declare Rank_Save(Filename.s)

Declare Rank_Select(Rank, Exact=0)

Declare Rank_Get_Pointer(Rank, Exact=0)

Declare Rank_Add(Rank, Name.s, Prefix.s, Suffix.s)

Declare Rank_Delete(Rank, Exact=0)

Declare Rank_Get_On_Client(Rank, Exact=0)

Declare Rank_Main()

Declare Map_HTML_Stats()

Declare Map_Select_ID(Map_ID, Log=0)

Declare Map_Select_Unique_ID(Unique_ID.s, Log=1)

Declare Map_Select_Name(Name.s)

Declare Map_Get_Pointer(Map_ID)

Declare Map_Get_ID()

Declare.s Map_Get_Unique_ID()

Declare.s Map_Get_MOTD_Override(Map_ID)

Declare Map_List_Save(Filename.s)

Declare Map_List_Load(Filename.s)

Declare Map_Settings_Save(Filename.s)

Declare Map_Settings_Load(Filename.s)

Declare Map_Add(Map_ID, X, Y, Z, Name.s)

Declare Map_Delete(Map_ID)

Declare Map_Resize(Map_ID, X, Y, Z) ; Ändert die Größe der Karte

Declare Map_Fill(Map_ID, Function_Name.s, Argument_String.s) ; Füllt die Karte mit einer Landschaft

Declare Map_Save(*Map_Data_Element.Map_Data, Directory.s) ; Komprimiert und Speichert die Karte (Gewählt über Pointer zum Element) (Thread)

Declare Map_Load(Map_ID, Directory.s) ; Dekomprimiert und lädt die Informationen in die aktuelle Karte / Decompresses and loads the information into the current map

Declare Map_Reload(Map_ID)

Declare Map_Unload(Map_ID)

Declare Map_Send(Client_ID, Map_ID)        ; Komprimiert und sendet die Karte an Client / Compresses and sends the map to the client

Declare Map_Export(Map_ID, X_0, Y_0, Z_0, X_1, Y_1, Z_1, Filename.s)

Declare Map_Import_Player(Player_Number, Filename.s, Map_ID, X, Y, Z, SX, SY, SZ) ; Dekomprimiert und importiert die Karte an X, Y, Z

Declare Map_Resend(Map_ID) ; Sendet die Karte an alle neu / Sends the map to all new clients

Declare Map_Action_Add_Save(Client_ID, Map_ID, Directory.s)

Declare Map_Action_Add_Load(Client_ID, Map_ID, Directory.s)

Declare Map_Action_Add_Resize(Client_ID, Map_ID, X, Y, Z)

Declare Map_Action_Add_Fill(Client_ID, Map_ID, Function_Name.s, Argument_String.s)

Declare Map_Action_Add_Delete(Client_ID, Map_ID)

Declare Map_Action_Thread(*Dummy)

Declare Map_Env_Colors_Change(*Map_Data.Map_Data, Red, Green, Blue, Type)

Declare Map_Env_Appearance_Set(*Map_Data.Map_Data, Texture.s, Side_Block, Edge_Block, Side_Level.w)

Declare Map_HackControl_Set(*Map_Data.Map_Data, Flying, NoClip, Speeding, SpawnControl, ThirdPerson, JumpHeight.w)

Declare Map_Block_Changed_Add(*Map_Data.Map_Data, X, Y, Z, Priority.a, Old_Material.w)

Declare Map_Block_Change(Player_Number, *Map_Data.Map_Data, X, Y, Z, Type.a, Undo.a, Physic.a, Send.a, Send_Priority.a) ; Blockänderung, kein prüfen des Ranks

Declare Map_Block_Change_Client(*Client.Network_Client, *Map_Data.Map_Data, X, Y, Z, Mode.a, Type.a) ; Blockänderung durch Klient, prüfen des Ranks (+Nachricht)

Declare Map_Block_Change_Player(*Player.Player_List, *Map_Data.Map_Data, X, Y, Z, Type.a, Undo.a, Physic.a, Send.a, Send_Priority.a) ; Blockänderung eines Players, prüfen des Ranks

Declare Map_Block_Move(*Map_Data.Map_Data, X_0, Y_0, Z_0, X_1, Y_1, Z_1, Undo, Physic, Send_Priority)

Declare Map_Block_Get_Type(*Map_Data.Map_Data, X.l, Y.l, Z.l)

Declare Map_Block_Get_Rank(*Map_Data.Map_Data, X.l, Y.l, Z.l)

Declare Map_Block_Get_Player_Number(*Map_Data.Map_Data, X.l, Y.l, Z.l)

Declare Map_Block_Set_Rank_Box(*Map_Data.Map_Data, X_0, Y_0, Z_0, X_1, Y_1, Z_1, Rank.w)

Declare Map_Block_Do_Add(*Map_Data.Map_Data, X.l, Y.l, Z.l) ; Fügt einen Block in die Abarbeitungsschleife ein (Filtert blöcke, welche nichts tun)

Declare Map_Block_Do_Distribute(*Map_Data.Map_Data, X, Y, Z)

Declare Map_Physic_Thread(*Dummy) ; Thread, für Physik / Thread for Physics

Declare Map_Blockchanging_Thread(*Dummy) ; In diesem Thread werden alle Blockänderungen nacheinander gesendet / Sends all blockchanges, sequentially.

Declare Map_Main()

Declare Map_Overview_Save_2D(*Map_Data_Element.Map_Data, Directory.s) ; Speichert ein Abbild der Karte als Image

Declare Map_Overview_Save_Iso_Fast(*Map_Data_Element.Map_Data, Filename.s) ; Speichert ein Abbild der Karte als Image (Isometrisch)

Declare Build_Line_Player(Player_Number, Map_ID, X_0, Y_0, Z_0, X_1, Y_1, Z_1, Material, Priority, Undo, Physic) ; - Builds a line between two points.

Declare Build_Box_Player(Player_Number, Map_ID, X_0, Y_0, Z_0, X_1, Y_1, Z_1, Material, Replace_Material, Hollow, Priority, Undo, Physic) ; - Fills the area between two points.

Declare Build_Sphere_Player(Player_Number, Map_ID, X, Y, Z, R.f, Material, Replace_Material, Hollow, Priority, Undo, Physic)

Declare Build_Rank_Box(Map_ID, X_0, Y_0, Z_0, X_1, Y_1, Z_1, Rank, Max_Rank) ; Creates a rank-box

Declare Build_Queue_Do()

Declare Build_Main()

Declare BuildShutdown()

Declare Physic_Block_Fill_Array_Clear() ; Löscht den Inhalt des Arrays

Declare Physic_Block_Compute_10(*Map_Data.Map_Data, X.l, Y.l, Z.l) ; Berechnet einen Block Mit Physic=10

Declare Physic_Block_Compute_11(*Map_Data.Map_Data, X.l, Y.l, Z.l) ; Berechnet einen Block Mit Physic=11

Declare Physic_Block_Compute_20(*Map_Data.Map_Data, X.l, Y.l, Z.l) ; Berechnet einen Block Mit Physic=20

Declare Physic_Block_Compute_21(*Map_Data.Map_Data, X.l, Y.l, Z.l) ; Berechnet einen Block Mit Physic=21

Declare Player_List_Database_Close() ; Schließt die Datenbank.

Declare Player_List_Database_Create(Filename.s) ; Erstellt eine Datenbank und richtet die Grundtabellen ein

Declare Player_List_Database_Open(Filename.s) ; Öffnet eine Datenbank, es kann nun geschrieben und gelesen werden.

Declare Player_List_Select(Name.s, Log=1) ; Wählt das Linked-List-Objekt

Declare Player_List_Select_Number(Number, Log=1) ; Wählt das Linked-List-Objekt

Declare Player_List_Get_Pointer(Number, Log=1) ; Wählt das Linked-List-Objekt

Declare Player_List_Get_Number() ; Eindeutige Spielernummer

Declare Player_List_Load(Filename.s) ; Lädt die Liste mit Spielern aus Datenbank

Declare Player_List_Save(Filename.s) ; Speichert/Überschreibt die Liste mit (Geänderten)Spielern in Datenbank

Declare Player_List_Save_Old(Filename.s) ; Speichert die Liste mit Spielern

Declare Player_List_Load_Old(Filename.s) ; Lädt die Liste mit Spielern (Alt)

Declare Player_List_Add(Name.s) ; Fügt ein Player zur Liste hinzu, wenn noch nicht vorhanden

Declare Player_List_Main()

Declare Player_Save(Filename.s) ; Speichert die Einstellungen

Declare Player_Load(Filename.s) ; Lädt die Einstellungen

Declare Player_Attribute_Long_Set(Player_Number, Attribute.s, Value.l) ; Speichert ein Attribut (Long)

Declare Player_Attribute_Long_Get(Player_Number, Attribute.s) ; Gibt ein Attribut (Long) aus

Declare Player_Attribute_String_Set(Player_Number, Attribute.s, String.s) ; Speichert ein Attribut (String)

Declare.s Player_Attribute_String_Get(Player_Number, Attribute.s) ; Gibt ein Attribut (String) aus

Declare.s Player_Inventory_Set(Player_Number, Material, Number) ; Setzt den Inventarswert

Declare Player_Inventory_Get(Player_Number, Material) ; Gibt den Inventarswert aus

Declare Player_Rank_Set(Player_Number, Rank, Reason.s)

Declare Player_Kick(Player_Number, Reason.s, Count, Log, Show) ; Kickt alle Klienten des Spielers

Declare Player_Global_Set(Player_Number, value)

Declare Player_Global_Get(Player_Number)

Declare Player_Ban(Player_Number, Reason.s) ; Bannt den Spieler und Kickt alle Klienten

Declare Player_Unban(Player_Number) ; Entbannt den Spieler

Declare Player_Stop(Player_Number, Reason.s) ; Stoppt den Spieler

Declare Player_Unstop(Player_Number) ; Entstoppt den Spieler

Declare Player_Mute(Player_Number, Minutes, Reason.s) ; Stellt den Spieler stumm

Declare Player_Unmute(Player_Number) ; Entstummt den Spieler

Declare Player_Get_Online(Player_Number, ID_Exception)

Declare.s Player_Get_Prefix(Player_Number)

Declare.s Player_Get_Name(Player_Number)

Declare.s Player_Get_Suffix(Player_Number)

Declare Player_Ontime_Counter_Add(Seconds.d) ; Erhöht den Sekunden Zähler aller Spieler(Online) in der Playerlist

Declare Player_Main()

Declare Client_Login(Client_ID, Name.s, MPPass.s, Version) ; A new player has logged in, everything important is created.

Declare Client_Logout(Client_ID, Message.s, Show_2_All) ; Player has logged out, everything important is sent out and done.

Declare Client_Login_Thread(*Dummy) ; In this thread, all logins are processed sequentially.

Declare.s HandleChatEscapes(Input.s)

Declare Chat_Message_Network_Send_2_Map(Entity_ID, Message.s) ; Sends a message to all clients of an entity on a map.

Declare Chat_Message_Network_Send_2_All(Entity_ID, Message.s) ; Sends a message to all clients of a map (Global Chat)

Declare Chat_Message_Network_Send(Entity_ID, Player_Name.s, Message.s) ; Sends a message from one entity to another.

Declare HandleIncomingChat(Text.s, PlayerId.b)

Declare Build_Mode_Load(Filename.s)

Declare Build_Mode_Save(Filename.s)

Declare Build_Mode_Distribute(Client_ID, Map_ID, X, Y, Z, Mode, Block_Type.a)

Declare Build_Mode_Blocks_Resend(Client_ID)

Declare Build_Mode_Set(Client_ID, Build_Mode.s)

Declare.s Build_Mode_Get(Client_ID)

Declare Build_Mode_State_Set(Client_ID, Build_State)

Declare Build_Mode_State_Get(Client_ID)

Declare Build_Mode_Coordinate_Set(Client_ID, Index, X, Y, Z)

Declare Build_Mode_Coordinate_Get_X(Client_ID, Index)

Declare Build_Mode_Coordinate_Get_Y(Client_ID, Index)

Declare Build_Mode_Coordinate_Get_Z(Client_ID, Index)

Declare Build_Mode_Long_Set(Client_ID, Index, Value)

Declare Build_Mode_Long_Get(Client_ID, Index)

Declare Build_Mode_Float_Set(Client_ID, Index, Value.f)

Declare.f Build_Mode_Float_Get(Client_ID, Index)

Declare Build_Mode_String_Set(Client_ID, Index, Value.s)

Declare.s Build_Mode_String_Get(Client_ID, Index)

Declare Build_Mode_Main()

Declare BuildModeShutdown()

Declare Client_Count_Elements()

Declare Client_Get_Array(*Memory)

Declare Network_Settings_Get_Port()

Declare Entity_Count_Elements()

Declare Entity_Get_Array(*Memory)

Declare Player_Count_Elements()

Declare Player_Get_Array(*Memory)

Declare Player_Get_Players_Max()

Declare Map_Count_Elements()

Declare Map_Get_Array(*Memory)

Declare Block_Count_Elements()

Declare Block_Get_Array(*Memory)

Declare Rank_Count_Elements()

Declare Rank_Get_Array(*Memory)

Declare Teleporter_Count_Elements(*Map_Data.Map_Data)

Declare Teleporter_Get_Array(*Map_Data.Map_Data, *Memory)

Declare.s System_Get_Server_Name()

Declare Main_LockMutex()

Declare Main_UnlockMutex()

Declare Plugin_Event_Block_Physics(Destination.s, *Map_Data.Map_Data, X, Y, Z)

Declare Plugin_Event_Block_Create(Destination.s, *Map_Data.Map_Data, X, Y, Z, Old_Block.a, *Client.Network_Client)

Declare Plugin_Event_Block_Delete(Destination.s, *Map_Data.Map_Data, X, Y, Z, Old_Block.a, *Client.Network_Client)

Declare Plugin_Event_Map_Fill(Destination.s, *Map_Data.Map_Data, Argument_String.s)

Declare Plugin_Event_Command(Destination.s, *Client.Network_Client, Command.s, Text_0.s, Text_1.s, Arg_0.s, Arg_1.s, Arg_2.s, Arg_3.s, Arg_4.s)

Declare Plugin_Event_Build_Mode(Destination.s, *Client.Network_Client, *Map_Data.Map_Data, X, Y, Z, Mode, Block_Type)

Declare PluginBuildModeGet(ClientID, *Result)

Declare PluginBuildModeStringGet(Client_ID, Index, *Result)

Declare PluginEntityDisplaynameGet(ID, *Result)

Declare PluginPlayerAttributeStringGet(Player_Number, Attribute.s, *Result)

Declare PluginSystemGetServerName(*Result)

Declare PluginLangGet(Language.s, Input.s, *Result, Field_0.s = "", Field_1.s = "", Field_2.s = "", Field_3.s = "")

Declare PluginFilesFileGet(File.s, *Result)

Declare PluginFilesFolderGet(Name.s, *Result)

Declare Plugin_Event_Client_Add(*Client.Network_Client)

Declare Plugin_Event_Client_Delete(*Client.Network_Client)

Declare Plugin_Event_Client_Verify_Name(Name.s, Pass.s)

Declare Plugin_Event_Client_Login(*Client.Network_Client)

Declare Plugin_Event_Client_Logout(*Client.Network_Client)

Declare Plugin_Event_Entity_Add(*Entity.Entity)

Declare Plugin_Event_Entity_Delete(*Entity.Entity)

Declare Plugin_Event_Entity_Position_Set(*Entity.Entity, Map_ID, X.f, Y.f, Z.f, Rotation.f, Look.f, Priority.a, Send_Own_Client.a)

Declare Plugin_Event_Entity_Map_Change(Client_ID, New_Map_ID, Old_Map_ID)

Declare Plugin_Event_Entity_Die(*Entity.Entity)

Declare Plugin_Event_Map_Add(*Map_Data.Map_Data)

Declare Plugin_Event_Map_Action_Delete(Action_ID, *Map_Data.Map_Data)

Declare Plugin_Event_Map_Action_Resize(Action_ID, *Map_Data.Map_Data)

Declare Plugin_Event_Map_Action_Fill(Action_ID, *Map_Data.Map_Data)

Declare Plugin_Event_Map_Action_Save(Action_ID, *Map_Data.Map_Data)

Declare Plugin_Event_Map_Action_Load(Action_ID, *Map_Data.Map_Data)

Declare Plugin_Event_Map_Block_Change(Player_Number, *Map_Data.Map_Data, X, Y, Z, Type.a, Undo.a, Physic.a, Send.a, Send_Priority.a)

Declare Plugin_Event_Map_Block_Change_Client(*Client.Network_Client, *Map_Data.Map_Data, X, Y, Z, Mode.a, Type.a)

Declare Plugin_Event_Map_Block_Change_Player(*Player.Player_List, *Map_Data.Map_Data, X, Y, Z, Type.a, Undo.a, Physic.a, Send.a, Send_Priority.a)

Declare Plugin_Event_Chat_Map(*Entity.Entity, Message.s)

Declare Plugin_Event_Chat_All(*Entity.Entity, Message.s)

Declare Plugin_Event_Chat_Private(*Entity.Entity, Player_Name.s, Message.s) ; ####################### Not Finished !!!!!!!!!!!!!

Declare Plugin_Initialize(Filename.s) ; Initialisiert Plugin und übergibt Funktionspointer...

Declare Plugin_Deinitialize(Filename.s) ; Deinitialisiert Plugin...

Declare Plugin_Unload(Filename.s) ; Entlädt die Lib, löscht sie aber nicht aus der Liste / Unloads plugin but does not remove from list

Declare Plugin_Load(Filename.s) ; Lädt die Lib (Wenn in der Liste vorhanden) / Loads the library (If its in the list)

Declare Plugin_Check_Files(Directory.s)

Declare Plugin_Main()

Declare Plugin_Thread(*Dummy)

Declare Command_Load(Filename.s)

Declare Command_Save(Filename.s)

Declare Command_Global()

Declare Command_Kick() ; Kicks a player (refers to Player_List ())

Declare Command_Ban() ; bans a player (refers to Player_List ())

Declare Command_Unban() ; (refers to Player_List ())

Declare Command_Stop() ; (refers to Player_List ())

Declare Command_Unstop() ; (refers to Player_List ())

Declare Command_Mute() ; (refers to Player_List ())

Declare Command_Unmute() ; (refers to Player_List ())

Declare Command_Commands()

Declare Command_Command_Help()

Declare Command_Players()

Declare Command_Player_Info()

Declare Command_Player_Attribute_Long_Set()

Declare Command_Player_Attribute_Long_Get()

Declare Command_Player_Attribute_String_Set()

Declare Command_Player_Attribute_String_Get()

Declare Command_Setrank()

Declare Command_Getrank()

Declare Command_Material()

Declare Command_Materials()

Declare Command_Place()

Declare Command_Undo_Time()

Declare Command_Undo_Player()

Declare Command_Undo()

Declare Command_Map_Info()

Declare Command_Map_Save()

Declare Command_Map_Load()

Declare Command_Map_Resize()

Declare Command_Map_Rename()

Declare Command_Map_Directory_Rename()

Declare Command_Map_Delete()

Declare Command_Map_Add()

Declare Command_Map_Fill()

Declare Command_Map_Blocks_Count()

Declare Command_Map_Rank_Build_Set()

Declare Command_Map_Rank_Join_Set()

Declare Command_Map_Rank_Show_Set()

Declare Command_Map_Physic_Stop()

Declare Command_Map_Physic_Start()

Declare Command_Map_Change()

Declare Command_Maps()

Declare Command_User_Maps()

Declare Command_Teleport()

Declare Command_Bring()

Declare Command_Setspawn()

Declare Command_Setkillspawn()

Declare Command_Set_Location()

Declare Command_Delete_Location()

Declare Command_Teleport_Location()

Declare Command_Bring_Location()

Declare Command_Locations()

Declare Command_Teleporters()

Declare Command_Delete_Teleporter()

Declare Command_Time()

Declare Command_Server_Info()

Declare Command_Log_Last()

Declare Command_Ping()

Declare Command_Watchdog()

Declare Command_Plugins()

Declare Command_Plugin_Load()

Declare Command_Plugin_Unload()

Declare Command_Crash()

Declare Command_Do(Client_ID, Input.s) ; Parses and passes the command on to the function.

Declare Command_Main()

Declare TMessage_Load(Filename.s)

Declare TMessage_Save(Filename.s)

Declare TMessage_Do() ; Sendet eine Nachricht

Declare TMessage_Main()

Declare Font_Load(Filename.s)

Declare Font_Select_ID(Font_ID.s)

Declare Font_Draw_Character(Player_Number, Font_ID.s, Map_ID, X, Y, Z, V_X.f, V_Y.f, Char.a, Material_F, Material_B)

Declare Font_Draw_Character_Player(*Player.Player_List, Font_ID.s, Map_ID, X, Y, Z, V_X.f, V_Y.f, Char.a, Material_F, Material_B)

Declare Font_Draw_Text(Player_Number, Font_ID.s, Map_ID, X, Y, Z, V_X.f, V_Y.f, String.s, Material_F, Material_B)

Declare Font_Draw_Text_Player(*Player.Player_List, Font_ID.s, Map_ID, X, Y, Z, V_X.f, V_Y.f, String.s, Material_F, Material_B)

Declare Font_Main()

Declare Undo_Save(Filename.s) ; Speichert die Einstellungen / Saves Settings

Declare Undo_Load(Filename.s) ; Lädt die Einstellungen / Loads settings

Declare Undo_Add(Player_Number, Map_ID, X, Y, Z, Type_Before.b, Player_Before) ; Adds a step to the change tracking system

Declare Undo_Do_Player(Map_ID, Player_Number, Time) ; Macht alle Änderungen von einem bestimmten Player rückgängig. / Undoes changes made by a particular player

Declare Undo_Do_Time(Map_ID, Time) ; Stellt alle Blöcke von einem bestimmten Zeitraum wieder her. / Undoes block changes based on time.

Declare Undo_Clear_Map(Map_ID) ; Löscht Undo-Schritte einer Map / Removes all undo steps for a given map

Declare Undo_Clear() ; Löscht ältere Undo-Schritte / Removes all undo steps.

Declare Undo_Main()

Declare System_Login_Screen(Client_ID, Message_0.s, Message_1.s, Op_Mode)

Declare System_Red_Screen(Client_ID, Message.s)

Declare System_Message_Network_Send(Client_ID, Message.s, Type=0) ; Sendet eine Nachricht zu einem Clienten

Declare System_Message_Network_Send_2_All(Map_ID, Message.s, Type=0) ; Sendet eine Nachricht zu allen Clienten

Declare Network_Out_Block_Set(Client_ID, X, Y, Z, Type.a) ; Sendet einen Block zu "Client_ID"

Declare Network_Out_Block_Set_2_Map(Map_ID, X, Y, Z, Type.a) ; Sendet einen Block zu allen Clienten einer Karte

Declare Network_Out_Entity_Add(Client_ID, ID_Client, Name.s, X.f, Y.f, Z.f, Rotation.f, Look.f) ; Erstellt ein neues Spielerobjekt auf einem Client

Declare Network_Out_Entity_Delete(Client_ID, ID_Client) ; Löscht eine Spielerobjekt auf einem Client

Declare Network_Out_Entity_Position(Client_ID, ID_Client, X.f, Y.f, Z.f, Rotation.f, Look.f) ; Sendet die Spielerbewegung von "Client_ID" zu allen

Declare Entity_Select_ID(ID, Log=1)

Declare Entity_Select_Name(Name.s, Log=1)

Declare Entity_Get_Pointer(ID)

Declare Entity_Get_Free_ID()

Declare Entity_Get_Free_ID_Client(Map_ID)

Declare Entity_Add(Name.s, Map_ID, X.f, Y.f, Z.f, Rotation.f, Look.f)

Declare Entity_Delete(ID)

Declare Entity_Resend(ID)

Declare Entity_Message_2_Clients(ID, Message.s) ; Sendet eine Nachricht zu den Mutterklienten

Declare.s Entity_Displayname_Get(ID)

Declare Entity_Displayname_Set(ID, Prefix.s, Name.s, Suffix.s)

Declare Entity_Kill(ID)

Declare Entity_Position_Check(ID) ; Prüft, ob dieses Entity den Block betreten darf/ ob er tötlich ist ... und Teleporter

Declare Entity_Position_Set(ID, Map_ID, X.f, Y.f, Z.f, Rotation.f, Look.f, Priority.a, Send_Own_Client.a)

Declare Entity_Send() ; Maintained moving, creating and deleting entities of the client

Declare Entity_Main()

Declare Hotkey_Remove(Label.s)

Declare Hotkey_Add(Label.s, Action.s, Keycode.l, Keymods.b)

Declare Hotkeys_Load(Filename.s)

Declare Hotkeys_Save(Filename.s)

Declare Hotkey_Main()

Declare List_Contains(Value.b, List mList.b())

Declare List_PopPosition(Value.b, List mList.b()) ; Sets the list position to the given element

Declare CPE_Send_ExtInfo(Client_ID, Name.s, MPPass.s, Version)

Declare CPE_Send_Extensions(Client_ID)

Declare.s Emote_Replace(Message.s)

Declare CPE_GetHeldBlock(Client_ID)

Declare CPE_HoldThis(Client_ID, Block, CanChange)

Declare CPE_Selection_Cuboid_Add(Client_ID, SelectionID, Label.s, StartX.w, StartY.w, StartZ.w, EndX.w, EndY.w, EndZ.w, Red.w, Green.w, Blue.w, Opacity.w)

Declare CPE_Selection_Cuboid_Delete(Client_ID, Selection_ID)

Declare.i Map_Export_Get_Size_X(Filename.s)

Declare.i Map_Export_Get_Size_Y(Filename.s)

Declare.i Map_Export_Get_Size_Z(Filename.s)

Declare CPE_Model_Change(Client_ID, Model.s)

Declare CPE_Aftermap_Actions(Client_ID, *MapData.Map_Data)

Declare CPEAddExtPlayer()

Declare CPE_Set_Env_Colors(Type.b, Red.w, Green.w, Blue.w)

Declare CPE_Set_Weather(Client_ID, Weather.b)

Declare CPE_Handle_Entity()

Declare CPE_Client_Set_Block_Permissions(Client_ID, Block_ID, CanPlace, CanDelete)

Declare CPE_Client_Send_Map_Appearence(Client_ID, URL.s, Side_Block, Edge_Block, Side_Level.w)

Declare CPE_Client_Send_Hotkeys(Client_ID)

Declare CPE_Client_Hackcontrol_Send(Client_ID, Flying, Noclip, Speeding, SpawnControl, ThirdPerson, Jumpheight.w)

Declare CPE_GetClientExtVersion(Extension.s)

