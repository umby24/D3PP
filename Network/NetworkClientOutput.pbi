; NetworkClientOutput.pbi
; Contains all of the functions writing to the outgoing packet buffer for network clients.
;TODO: A lot of this is duplicate stuff related to ringbuffers..

Procedure.l Network_Client_Output_Available(*Client.Network_Client)     ; Returns the number of bytes in the output buffer.
    Protected Return_Value.l
    Return_Value =  *Client\Buffer_Output_Available
    ProcedureReturn Return_Value
EndProcedure

Procedure Network_Client_Output_Add_Offset(*Client.Network_Client, Bytes.l)     ; Addiert einige Bytes zum Offset des Sendebuffers
    List_Store(*Network_Client_Old, Network_Client())
    
    *Client\Buffer_Output_Offset + Bytes
    *Client\Buffer_Output_Available - Bytes
    
    If *Client\Buffer_Output_Offset < 0
        *Client\Buffer_Output_Offset + #Network_Buffer_Size
    EndIf
    
    If *Client\Buffer_Output_Offset >= #Network_Buffer_Size
        *Client\Buffer_Output_Offset - #Network_Buffer_Size
    EndIf
EndProcedure

Procedure Network_Client_Output_Read_Buffer(*Client.Network_Client, *Data_Buffer, Data_Size.l)   ; Liest Daten aus dem Sendebuffer
    Protected Data_Read.l, Ringbuffer_Max_Data.l, *Ringbuffer_Adress, Data_Temp_Size

    ; Anzahl gelesener Daten
    Data_Read = 0
    
    While Data_Read < Data_Size
        
        ; Square to the "end" of the ring buffer
        Ringbuffer_Max_Data = #Network_Buffer_Size - (*Client\Buffer_Output_Offset)
        ; Buffer Address with offset
        *Ringbuffer_Adress = *Client\Buffer_Output + *Client\Buffer_Output_Offset
        ; Temporary data to be read
        Data_Temp_Size = Data_Size - Data_Read
        If Data_Temp_Size > Ringbuffer_Max_Data : Data_Temp_Size = Ringbuffer_Max_Data : EndIf
        
        CopyMemory(*Ringbuffer_Adress, *Data_Buffer + Data_Read, Data_Temp_Size)
        Data_Read + Data_Temp_Size
        *Client\Buffer_Output_Offset + Data_Temp_Size
        *Client\Buffer_Output_Available - Data_Temp_Size
        
        If *Client\Buffer_Output_Offset >= #Network_Buffer_Size
            *Client\Buffer_Output_Offset - #Network_Buffer_Size
        EndIf
    Wend
EndProcedure

Procedure Network_Client_Output_Write_Byte(*Client.Network_Client, Value.b)     ; Writes a byte to the send buffer
    ; Buffer address with offset.
    *Ringbuffer_Adress = *Client\Buffer_Output + ((*Client\Buffer_Output_Offset + *Client\Buffer_Output_Available) % #Network_Buffer_Size)
    
    PokeB(*Ringbuffer_Adress, Value)
    
    *Client\Buffer_Output_Available + 1
EndProcedure

Procedure Network_Client_Output_Write_Word(*Client.Network_Client, Value.w)     ; Write a short to the send buffer
    ; Bufferadresse mit Offset
    *Ringbuffer_Adress = *Client\Buffer_Output + ((*Client\Buffer_Output_Offset + *Client\Buffer_Output_Available) % #Network_Buffer_Size)
    
    PokeB(*Ringbuffer_Adress, Value << 8)
    PokeB(*Ringbuffer_Adress, Value)
    
    *Client\Buffer_Output_Available + 2
EndProcedure

Procedure Network_Client_Output_Write_Int(*Client.Network_Client, Value.l) ;Using 'Long' here, because in this context, it is an int (4 Bytes)
    *Ringbuffer_Adress = *Client\Buffer_Output + ((*Client\Buffer_Output_Offset + *Client\Buffer_Output_Available) % #Network_Buffer_Size)
    
    PokeB(*Ringbuffer_Adress, Value << 32)
    PokeB(*Ringbuffer_Adress, Value << 16)
    PokeB(*Ringbuffer_Adress, Value << 8)
    PokeB(*Ringbuffer_Adress, Value)
    
    *Client\Buffer_Output_Available + 4
EndProcedure

Procedure Network_Client_Output_Write_String(*Client.Network_Client, String.s, Length)     ; Write a string of the given length to the sendbuffer
    ; Anzahl geschriebener Daten
    Data_Wrote = 0
    
    While Data_Wrote < Length
        
        ; Position im Ringbuffer
        Ringbuffer_Write_Offset = (*Client\Buffer_Output_Offset + *Client\Buffer_Output_Available) % #Network_Buffer_Size
        ; Platz bis zum "ende" des Ringbuffers
        Ringbuffer_Max_Data = #Network_Buffer_Size - (Ringbuffer_Write_Offset)
        ; Bufferadresse mit Offset
        *Ringbuffer_Adress = *Client\Buffer_Output + Ringbuffer_Write_Offset
        ; Tempor?re zu schreibende Datenmenge
        Data_Temp_Size = Length - Data_Wrote
        If Data_Temp_Size > Ringbuffer_Max_Data : Data_Temp_Size = Ringbuffer_Max_Data : EndIf
        
        CopyMemory(@String + Data_Wrote, *Ringbuffer_Adress, Data_Temp_Size)
        
        Data_Wrote + Data_Temp_Size
        *Client\Buffer_Output_Available + Data_Temp_Size
    Wend
    
EndProcedure

Procedure Network_Client_Output_Write_Buffer(*Client.Network_Client, *Data_Buffer, Data_Size)     ; Write raw bytes into the send buffer.
    ; Anzahl geschriebener Daten
    Data_Wrote = 0
    
    While Data_Wrote < Data_Size
        
        ; Position im Ringbuffer
        Ringbuffer_Write_Offset = (*Client\Buffer_Output_Offset + *Client\Buffer_Output_Available) % #Network_Buffer_Size
        ; Platz bis zum "ende" des Ringbuffers
        Ringbuffer_Max_Data = #Network_Buffer_Size - (Ringbuffer_Write_Offset)
        ; Bufferadresse mit Offset
        *Ringbuffer_Adress = *Client\Buffer_Output + Ringbuffer_Write_Offset
        ; Tempor?re zu schreibende Datenmenge
        Data_Temp_Size = Data_Size - Data_Wrote
        If Data_Temp_Size > Ringbuffer_Max_Data : Data_Temp_Size = Ringbuffer_Max_Data : EndIf
        
        CopyMemory(*Data_Buffer + Data_Wrote, *Ringbuffer_Adress, Data_Temp_Size)
        
        Data_Wrote + Data_Temp_Size
        *Client\Buffer_Output_Available + Data_Temp_Size
    Wend
EndProcedure
; IDE Options = PureBasic 5.30 (Windows - x64)
; CursorPosition = 3
; Folding = --
; EnableUnicode
; EnableXP