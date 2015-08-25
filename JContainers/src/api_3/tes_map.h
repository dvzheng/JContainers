namespace tes_api_3 {

    using namespace collections;


    template<class Key
        , class Cnt
        , class key_ref = Key &
        , class key_cref = const Key &
    >
    class tes_map_t : public class_meta< tes_map_t<Key, Cnt, key_ref, key_cref> > {
    public:

        using map_functions = map_functions_templ < Cnt >;
        using map_type = Cnt;
        using tes_key = reflection::binding::get_converter_tes_type<typename map_type::key_type>;

        typedef typename Cnt* ref;

        tes_map_t() {
            metaInfo.comment = "Associative key-value container.\n"
                "Inherits JValue functionality";
        }

        REGISTERF(tes_object::object<Cnt>, "object", "", kCommentObject);

        template<class T>
        static T getItem(Cnt *obj, key_cref key, T def = T(0)) {
            map_functions::doReadOp(obj, key, [&](item& itm) { def = itm.readAs<T>(); });
            return def;
        }
        REGISTERF(getItem<SInt32>, "getInt", "object key default=0", "returns value associated with key");
        REGISTERF(getItem<Float32>, "getFlt", "object key default=0.0", "");
        REGISTERF(getItem<skse::string_ref>, "getStr", "object key default=\"\"", "");
        REGISTERF(getItem<object_base*>, "getObj", "object key default=0", "");
        REGISTERF(getItem<FormId>, "getForm", "object key default=None", "");

        template<class T>
        static void setItem(Cnt *obj, key_cref key, T val) {
            map_functions::doWriteOp(obj, key, [&](item& itm) { itm = val; });
        }
        REGISTERF(setItem<SInt32>, "setInt", "* key value", "creates key-value association. replaces existing value if any");
        REGISTERF(setItem<Float32>, "setFlt", "* key value", "");
        REGISTERF(setItem<const char *>, "setStr", "* key value", "");
        REGISTERF(setItem<object_base*>, "setObj", "* key container", "");
        REGISTERF(setItem<FormId>, "setForm", "* key value", "");

        static bool hasKey(ref obj, key_cref key) {
            return valueType(obj, key) != 0;
        }
        REGISTERF2(hasKey, "* key", "returns true, if something associated with key");

        static SInt32 valueType(ref obj, key_cref key) {
            auto type = item_type::no_item;
            map_functions::doReadOp(obj, key, [&](item& itm) { type = itm.type(); });
            return (SInt32)type;
        }
        REGISTERF2(valueType, "* key", "returns type of the value associated with key.\n"VALUE_TYPE_COMMENT);

        static object_base* allKeys(Cnt* obj) {
            if (!obj) {
                return nullptr;
            }

            return &array::objectWithInitializer([&](array &arr) {
                object_lock g(obj);

                arr._array.reserve(obj->u_count());
                for each(auto& pair in obj->u_container()) {
                    arr.u_container().emplace_back(pair.first);
                }
            },
                tes_context::instance());
        }
        REGISTERF(allKeys, "allKeys", "*", "returns new array containing all keys");

        static VMResultArray<tes_key> allKeysPArray(Cnt* obj) {
            if (!obj) {
                return VMResultArray<tes_key>();
            }

            VMResultArray<tes_key> keys;
            object_lock l(obj);
            keys.reserve(obj->u_count());
            std::transform(obj->u_container().begin(), obj->u_container().end(),
                std::back_inserter(keys),
                [](const typename map_type::value_type& p) {
                    return reflection::binding::get_converter<typename map_type::key_type>::convert2Tes(p.first);
                }
            );

            return keys;
        }
        REGISTERF2(allKeysPArray, "*", "");

        static object_base* allValues(Cnt *obj) {
            if (!obj) {
                return nullptr;
            }

            return &array::objectWithInitializer([&](array &arr) {
                object_lock g(obj);

                arr._array.reserve(obj->u_count());
                for each(auto& pair in obj->u_container()) {
                    arr._array.push_back(pair.second);
                }
            },
                tes_context::instance());
        }
        REGISTERF(allValues, "allValues", "*", "returns new array containing all values");

        static bool removeKey(Cnt *obj, key_cref key) {
            if (obj) {
                return obj->erase(key);
            }
            return false;
        }
        REGISTERF(removeKey, "removeKey", "* key", "destroys key-value association");

        static SInt32 count(ref obj) {
            if (!obj) {
                return 0;
            }

            return obj->s_count();
        }
        REGISTERF2(count, "*", "returns count of items/associations");

        static void clear(ref obj) {
            if (!obj) {
                return;
            }

            obj->s_clear();
        }
        REGISTERF2(clear, "*", "removes all items from container");

        static void addPairs(ref obj, const ref source, bool overrideDuplicates) {
            if (!obj || !source || source == obj) {
                return;
            }

            object_lock g(obj);
            object_lock c(source);

            if (overrideDuplicates) {
                for (const auto& pair : source->u_container()) {
                    obj->u_container()[pair.first] = pair.second;
                }
            }
            else {
                obj->u_container().insert(source->u_container().begin(), source->u_container().end());
            }
        }
        REGISTERF2(addPairs, "* source overrideDuplicates", "inserts key-value pairs from the source map");

        void additionalSetup();

        //////////////////////////////////////////////////////////////////////////

        static Key nextKey(ref obj, key_cref previousKey, key_ref endKey) {
            map_functions::nextKey(obj, previousKey, [&](const typename Cnt::key_type & key) { endKey = key; });
            return endKey;
        }

        static Key getNthKey(ref obj, SInt32 keyIndex) {
            Key ith;
            map_functions::getNthKey(obj, keyIndex, [&](const typename Cnt::key_type& key) { ith = key; });
            return ith;
        }
    };

    typedef tes_map_t<const char*, map, const char*, const char*> tes_map;
    typedef tes_map_t<weak_form_id, form_map> tes_form_map;
    typedef tes_map_t<int32_t, integer_map, int32_t, int32_t> tes_integer_map;

    void tes_map::additionalSetup() {
        metaInfo._className = "JMap";
    }

    void tes_form_map::additionalSetup() {
        metaInfo._className = "JFormMap";
    }

    void tes_integer_map::additionalSetup() {
        metaInfo._className = "JIntMap";
    }

    TES_META_INFO(tes_map);
    TES_META_INFO(tes_form_map);
    TES_META_INFO(tes_integer_map);

    //////////////////////////////////////////////////////////////////////////

    const char *tes_map_nextKey_comment =
        "Simplifies iteration over container's contents.\nIncrements and returns previous key, pass defaulf parameter to begin iteration. Usage:\n"
        "string key = JMap.nextKey(map)\n"
        "while key\n"
        "  <retrieve values here>\n"
        "  key = JMap.nextKey(map, key)\n"
        "endwhile";

    struct tes_map_ext : class_meta < tes_map_ext > {
        REGISTER_TES_NAME("JMap");
        template<class Key>
        static Key nextKey(map* obj, const char* previousKey = "", const char * endKey = "") {
            Key str(endKey);
            map_functions::nextKey(obj, previousKey, [&](const std::string& key) { str = key.c_str(); });
            return str;
        }
        REGISTERF(nextKey<skse::string_ref>, "nextKey", STR(* previousKey="" endKey=""), tes_map_nextKey_comment);

        static const char * getNthKey_comment() { return "Retrieves N-th key. " NEGATIVE_IDX_COMMENT "\nWorst complexity is O(n/2)"; }

        template<class Key>
        static Key getNthKey(map* obj, SInt32 keyIndex) {
            Key ith;
            map_functions::getNthKey(obj, keyIndex, [&](const std::string& key) { ith = key.c_str(); });
            return ith;
        }
        REGISTERF(getNthKey<skse::string_ref>, "getNthKey", "* keyIndex", getNthKey_comment());
    };

    struct tes_form_map_ext : class_meta < tes_form_map_ext > {
        REGISTER_TES_NAME("JFormMap");
        REGISTERF(tes_form_map::nextKey, "nextKey", STR(* previousKey=None endKey=None), tes_map_nextKey_comment);
        REGISTERF(tes_form_map::getNthKey, "getNthKey", "* keyIndex", tes_map_ext::getNthKey_comment());
    };

    struct tes_integer_map_ext : class_meta < tes_integer_map_ext > {
        REGISTER_TES_NAME("JIntMap");
        REGISTERF(tes_integer_map::nextKey, "nextKey", STR(* previousKey=0 endKey=0), tes_map_nextKey_comment);
        REGISTERF(tes_integer_map::getNthKey, "getNthKey", "* keyIndex", tes_map_ext::getNthKey_comment());
    };

    TES_META_INFO(tes_map_ext);
    TES_META_INFO(tes_form_map_ext);
    TES_META_INFO(tes_integer_map_ext);

}
