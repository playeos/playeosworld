#include "playeosworld.hpp"

#include <eosiolib/action.hpp>
#include <eosiolib/asset.hpp>

using namespace eosio;
using namespace std;

void playeosworld::init(const uint64_t &key, const uint64_t &group, uint64_t &parent, const uint64_t &grandParent, const uint64_t &price, const string &name)
{
    require_auth(_self);

    db_resources resources(_self, _self);

    auto keyItr = resources.find(key);
    auto parentItr = resources.find(parent);
    auto grandParentItr = resources.find(grandParent);

    eosio_assert(group >= 1 && group <= 3, "invalid group");
    eosio_assert(keyItr == resources.end(), "existing key");

    if (group == 1)
    {
        eosio_assert(parent == 0, "invalid group parent");
        eosio_assert(grandParent == 0, "invalid group grand parent");
    }
    else if (group == 2)
    {
        eosio_assert(parent > 0, "invalid group parent");
        eosio_assert(grandParent == 0, "invalid group grand parent");
        eosio_assert(parentItr != resources.end(), "invalid parent key");
    }
    else if (group == 3)
    {
        eosio_assert(parent > 0, "invalid group parent");
        eosio_assert(grandParent > 0, "invalid group grand parent");
        eosio_assert(parentItr != resources.end(), "invalid parent key");
        eosio_assert(grandParentItr != resources.end(), "invalid grand parent key");
    }

    resources.emplace(_self, [&](auto &resource) {
        resource.key = key;
        resource.group = group;
        resource.parent = parent;
        resource.grandParent = grandParent;
        resource.price = price * 1.35;
        resource.owner = _self;
        resource.name = name;
    });

    if (parent > 0)
    {
        resources.modify(parentItr, _self, [&](auto &resource) {
            resource.price = resource.price + price * 1.30;
        });
    }

    if (grandParent > 0)
    {
        resources.modify(grandParentItr, _self, [&](auto &resource) {
            resource.price = resource.price + price * 1.25;
        });
    }
}

void playeosworld::buy(const eosio::currency::transfer &transfer)
{
    require_auth(transfer.from);

    if (transfer.to != _self) {
        return;
    }

    eosio_assert(transfer.from != _self, "invalid buyer");
    eosio_assert(transfer.quantity.symbol == EOS_SYMBOL, "invalid token");

    uint64_t key = stoi(transfer.memo); 

    db_resources resources(_self, _self);

    auto keyItr = resources.find(key);
    eosio_assert(keyItr != resources.end(), "invalid key");
    
    auto keyResource = *keyItr;
    eosio_assert(transfer.quantity.amount == keyResource.price, "invalid price");

    auto parentItr = resources.find(keyResource.parent);
    auto parentResource = *parentItr;

    auto grandParentItr = resources.find(keyResource.grandParent);
    auto grandParentResource = *grandParentItr;

    uint64_t nextPrice = 0;
    uint64_t keyProfit = 0;
    uint64_t parentProfit = 0;
    uint64_t grandParentProfit = 0;

    if (keyResource.group == 1)
    {
        nextPrice = keyResource.price * 1.25;
        keyProfit = keyResource.price * 1.20 / 1.25;
    }
    else if (keyResource.group == 2)
    {
        nextPrice = keyResource.price * 1.30;
        keyProfit = keyResource.price * 1.20 / 1.30;
        parentProfit = keyResource.price * 0.05 / 1.30;
    }
    else if (keyResource.group == 3)
    {
        nextPrice = keyResource.price * 1.35;
        keyProfit = keyResource.price * 1.20 / 1.35;
        parentProfit = keyResource.price * 0.05 / 1.35;
        grandParentProfit = keyResource.price * 0.05 / 1.35;
    }

    resources.modify(keyItr, _self, [&](auto &resource) {
        resource.price = nextPrice;
        resource.owner = transfer.from;
    });

    if (keyProfit > 0 && keyResource.owner != _self)
    {
        action(
            permission_level{_self, N(active)},
            N(eosio.token),
            N(transfer),
            std::make_tuple(_self, keyResource.owner, asset(keyProfit, EOS_SYMBOL), std::string("PLAYEOS WORLD PROFIT"))
        ).send();
    }

    if (parentProfit > 0 && parentResource.owner != _self)
    {
        action(
            permission_level{_self, N(active)},
            N(eosio.token),
            N(transfer),
            std::make_tuple(_self, parentResource.owner, asset(parentProfit, EOS_SYMBOL), std::string("PLAYEOS WORLD PROFIT"))
        ).send();
    }

    if (grandParentProfit > 0 && grandParentResource.owner != _self)
    {
        action(
            permission_level{_self, N(active)},
            N(eosio.token),
            N(transfer),
            std::make_tuple(_self, grandParentResource.owner, asset(grandParentProfit, EOS_SYMBOL), std::string("PLAYEOS WORLD PROFIT"))
        ).send();
    }
}

void playeosworld::apply(account_name contract, account_name action)
{
    if (contract == N(eosio.token) && action == N(transfer))
    {
        buy(unpack_action_data<currency::transfer>());
        return;
    }

    if (contract != _self)
    {
        return;
    }

    auto &thiscontract = *this;
    switch (action)
    {
        EOSIO_API(playeosworld, (init))
    };
}

extern "C"
{
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        playeosworld world(receiver);
        world.apply(code, action);
        eosio_exit(0);
    }
}