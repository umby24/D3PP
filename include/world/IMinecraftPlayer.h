//
// Created by Wande on 6/30/2022.
//

#ifndef D3PP_IMINECRAFTPLAYER_H
#define D3PP_IMINECRAFTPLAYER_H
#include <string>

class Entity;

namespace D3PP::world {
    class IMinecraftPlayer {
    public:
        IMinecraftPlayer() = default;;

        virtual ~IMinecraftPlayer() = default;;

        virtual int GetId() = 0;

        virtual int GetRank() = 0;
        virtual int GetNameId() = 0;
        virtual int GetCustomBlockLevel() = 0;
        virtual std::string GetLoginName() = 0;
        virtual std::shared_ptr<Entity> GetEntity() = 0;
        virtual void Login() = 0;

        virtual void Logout() = 0;


    };
}

#endif //D3PP_IMINECRAFTPLAYER_H
