#include <eosiolib/eosio.hpp>
#include <eosiolib/currency.hpp>

#define EOS_SYMBOL S(4, EOS)

using namespace eosio;
using namespace std;

class playeosworld : public eosio::contract
{
  public:
    playeosworld(account_name self) : contract(self) {}

    /// @abi table
    struct resource
    {
        uint64_t key;
        uint64_t group;
        uint64_t parent;
        uint64_t grandParent;
        uint64_t price;
        account_name owner;
        string name;
        uint64_t primary_key() const { return key; }
        EOSLIB_SERIALIZE(resource, (key)(group)(parent)(grandParent)(price)(owner)(name))
    };

    /// @abi action
    void init(const uint64_t &key, const uint64_t &group, uint64_t &parent, const uint64_t &grandParent, const uint64_t &price, const string &name);

    void buy(const eosio::currency::transfer &transfer);

    void apply(account_name contract, action_name action);
    
    typedef eosio::multi_index<N(resource), resource> db_resources;
};
