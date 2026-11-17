#pragma once

#include <third_party/haxsdk/hax.h>

#include "localization.h"

// Russian names for the items of the "Items > SPAWN" list. The key is Item.itemName as the game
// stores it (English). Items that are not in the table keep their English name.

namespace Cheat
{
    struct ItemNameRu
    {
        Hax::WStringView    En;
        Hax::WStringView    Ru;
    };

    inline constexpr ItemNameRu g_ItemNamesRu[] =
    {
        // Carts
        { L"C.A.R.T.",                      L"Тележка (C.A.R.T.)" },
        { L"POCKET C.A.R.T.",               L"Карманная тележка" },
        { L"C.A.R.T. Cannon",               L"Тележка-пушка" },
        { L"C.A.R.T. Laser",                L"Тележка-лазер" },

        // Drones
        { L"Recharge Drone",                L"Дрон-зарядник" },
        { L"Feather Drone",                 L"Дрон-пёрышко" },
        { L"Indestructible Drone",          L"Дрон неуязвимости" },
        { L"Roll Drone",                    L"Ролл-дрон" },
        { L"Zero Gravity Drone",            L"Дрон невесомости" },

        // Guns
        { L"Gun",                           L"Пистолет" },
        { L"Shotgun",                       L"Дробовик" },
        { L"Pulse Pistol",                  L"Импульсный пистолет" },
        { L"Photon Blaster",                L"Фотонный бластер" },
        { L"Tranq Gun",                     L"Пистолет-транквилизатор" },
        { L"Boltzap",                       L"Электроболт" },
        { L"Human Gun",                     L"Человеческая пушка" },

        // Melee
        { L"Baseball Bat",                  L"Бейсбольная бита" },
        { L"Frying Pan",                    L"Сковородка" },
        { L"Inflatable Hammer",             L"Надувной молоток" },
        { L"Sledge Hammer",                 L"Кувалда" },
        { L"Sword",                         L"Меч" },
        { L"Prodzap",                       L"Электрошокер" },
        { L"Leaf Blower",                   L"Воздуходувка" },

        // Grenades and mines
        { L"Grenade",                       L"Граната" },
        { L"Duct Taped Grenades",           L"Гранаты на изоленте" },
        { L"Human Grenade",                 L"Человеческая граната" },
        { L"Shockwave Grenade",             L"Граната ударной волны" },
        { L"Stun Grenade",                  L"Оглушающая граната" },
        { L"Explosive Mine",                L"Взрывная мина" },
        { L"Shockwave Mine",                L"Мина ударной волны" },
        { L"Trapzap",                       L"Электроловушка" },

        // Health
        { L"Small Health Pack (25)",        L"Маленькая аптечка (25)" },
        { L"Medium Health Pack (50)",       L"Средняя аптечка (50)" },
        { L"Large Health Pack (100)",       L"Большая аптечка (100)" },
        { L"Defibro",                       L"Дефибриллятор" },

        // Orbs, staffs and gadgets
        { L"Recharge Orb",                  L"Сфера подзарядки" },
        { L"Zero Gravity Orb",              L"Сфера невесомости" },
        { L"Roll Staff",                    L"Ролл-посох" },
        { L"Void Staff",                    L"Посох пустоты" },
        { L"Zero Gravity Staff",            L"Посох невесомости" },
        { L"Phase Bridge",                  L"Фазовый мост" },
        { L"Energy Crystal",                L"Энергокристалл" },
        { L"Human Adventure Cable",         L"Приключенческий трос" },
        { L"Valuable Tracker",              L"Трекер ценностей" },
        { L"Extraction Tracker",            L"Трекер точки сбора" },
        { L"Duck Bucket",                   L"Ведро для утки" },
        { L"Rubber Duck",                   L"Резиновая уточка" },
        { L"Semibot Walkies",               L"Рации Семибота" },
        { L"Hauler",                        L"Грузчик" },
        { L"Scout",                         L"Разведчик" },

        // Upgrades
        { L"Health Upgrade",                L"Улучшение: здоровье" },
        { L"Stamina Upgrade",               L"Улучшение: выносливость" },
        { L"Strength Upgrade",              L"Улучшение: сила" },
        { L"Range Upgrade",                 L"Улучшение: дальность захвата" },
        { L"Sprint Speed Upgrade",          L"Улучшение: скорость бега" },
        { L"Extra Jump Upgrade",            L"Улучшение: доп. прыжок" },
        { L"Crouch Rest Upgrade",           L"Улучшение: отдых в приседе" },
        { L"Tumble Launch Upgrade",         L"Улучшение: запуск при падении" },
        { L"Tumble Wings Upgrade",          L"Улучшение: крылья при падении" },
        { L"Tumble Climb Upgrade",          L"Улучшение: карабканье" },
        { L"Map Player Count Upgrade",      L"Улучшение: игроки на карте" },
        { L"Death Head Battery Upgrade",    L"Улучшение: заряд головы" },
        { L"Throw Upgrade",                 L"Улучшение: сила броска" },
    };

    // Name to show in the menu for Item.itemName.
    inline Hax::WStringView LocalizeItemName(Hax::WStringView name, int lang)
    {
        if (lang != Lang_Ru)
            return name;

        for (const ItemNameRu& entry : g_ItemNamesRu)
        {
            if (entry.En == name)
                return entry.Ru;
        }
        return name;
    }

    // Alphabetical order for menu lists: case-insensitive, Cyrillic aware (ё sorts as е).
    inline bool MenuNameLess(Hax::WStringView a, Hax::WStringView b)
    {
        auto fold = [](wchar_t c) -> wchar_t
        {
            if (c >= L'A' && c <= L'Z')         return (wchar_t)(c + 0x20);
            if (c >= 0x0410 && c <= 0x042F)     return (wchar_t)(c + 0x20);    // А-Я
            if (c == 0x0401 || c == 0x0451)     return (wchar_t)0x0435;        // Ё, ё -> е
            return c;
        };

        const size_t n = Hax::Min(a.Length(), b.Length());
        for (size_t i = 0; i < n; ++i)
        {
            const wchar_t ca = fold(a[i]);
            const wchar_t cb = fold(b[i]);
            if (ca != cb)
                return ca < cb;
        }
        return a.Length() < b.Length();
    }
}
