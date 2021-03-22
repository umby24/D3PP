//
// Created by Wande on 3/17/2021.
//

#include "Entity.h"

int Entity::GetFreeId() {
    int id = 0;
    bool found = false;
    while (true) {
        found = (_entities.find(id) != _entities.end());

        if (found)
            id++;
        else {
            return id;
        }
    }

}
