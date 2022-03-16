//
// Created by unknown on 3/16/2022.
//

#ifndef D3PP_IMINECRAFTPLAYER_H
#define D3PP_IMINECRAFTPLAYER_H
class IMinecraftPlayer {
public:
    IMinecraftPlayer()= default;;
    virtual ~IMinecraftPlayer()= default;;

    virtual int GetId() = 0;
    virtual int GetRank() = 0;
    virtual int GetCustomBlockLevel() = 0;
    void Login() = 0;
    void Logout() = 0;


};
#endif //D3PP_IMINECRAFTPLAYER_H
