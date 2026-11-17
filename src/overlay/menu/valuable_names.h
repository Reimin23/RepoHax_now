#pragma once

#include <third_party/haxsdk/hax.h>

// Russian names for the valuables of "Valuables > SPAWN VALUABLES". The key is the prefab name
// without "Valuable " (REPO v0.5.1), the first word is the level theme. Valuables that are not in
// the table keep their English name.

namespace Cheat
{
    struct ValuableThemeRu
    {
        Hax::WStringView    En;
        Hax::WStringView    Ru;
    };

    inline constexpr ValuableThemeRu g_ValuableThemesRu[] =
    {
        { L"Arctic",    L"Арктика" },
        { L"Desert",    L"Пустыня" },
        { L"Manor",     L"Особняк" },
        { L"Museum",    L"Музей" },
        { L"Wizard",    L"Академия" },
    };

    struct ValuableNameRu
    {
        Hax::WStringView    En;     // full name, theme included
        Hax::WStringView    Ru;     // name only, the theme is added from g_ValuableThemesRu
    };

    inline constexpr ValuableNameRu g_ValuableNamesRu[] =
    {
        // McJannek Station
        { L"Arctic 3D Printer",             L"3D-принтер" },
        { L"Arctic Barrel",                 L"Бочка" },
        { L"Arctic Big Sample",             L"Большой образец" },
        { L"Arctic Bonsai",                 L"Бонсай" },
        { L"Arctic Calculator",             L"Калькулятор" },
        { L"Arctic Camera",                 L"Фотоаппарат" },
        { L"Arctic Centrifuge",             L"Центрифуга" },
        { L"Arctic Coffee Cup",             L"Кофейная кружка" },
        { L"Arctic Computer",               L"Компьютер" },
        { L"Arctic Creature Leg",           L"Нога существа" },
        { L"Arctic Cryo Pod",               L"Криокапсула" },
        { L"Arctic Eraser",                 L"Ластик" },
        { L"Arctic Fan",                    L"Вентилятор" },
        { L"Arctic Fire Extinguisher",      L"Огнетушитель" },
        { L"Arctic Flamethrower",           L"Огнемёт" },
        { L"Arctic Flashlight",             L"Фонарик" },
        { L"Arctic Guitar",                 L"Гитара" },
        { L"Arctic HDD",                    L"Жёсткий диск" },
        { L"Arctic Heavy Water",            L"Тяжёлая вода" },
        { L"Arctic Ice Block",              L"Ледяная глыба" },
        { L"Arctic Ice Saw",                L"Ледовая пила" },
        { L"Arctic Icepick",                L"Ледоруб" },
        { L"Arctic Jackhammer",             L"Отбойный молоток" },
        { L"Arctic Keycard",                L"Ключ-карта" },
        { L"Arctic Laptop",                 L"Ноутбук" },
        { L"Arctic Phone",                  L"Телефон" },
        { L"Arctic Pills",                  L"Таблетки" },
        { L"Arctic Propane Tank",           L"Баллон с пропаном" },
        { L"Arctic Sample",                 L"Образец" },
        { L"Arctic Sample Cooler",          L"Контейнер для образцов" },
        { L"Arctic Sample Cooler Wide",     L"Широкий контейнер для образцов" },
        { L"Arctic Sample Six Pack",        L"Шесть образцов" },
        { L"Arctic Scale",                  L"Весы" },
        { L"Arctic Science Station",        L"Научная станция" },
        { L"Arctic Server Rack",            L"Серверная стойка" },
        { L"Arctic Smartwatch",             L"Умные часы" },
        { L"Arctic Snow Bike",              L"Снегоход" },
        { L"Arctic Stapler",                L"Степлер" },
        { L"Arctic Uranium Petri Dish",     L"Урановая чашка Петри" },
        { L"Arctic Usb stick",              L"Флешка" },
        { L"Arctic VHS",                    L"Видеокассета" },

        // Sunstone Valley Resort
        { L"Desert Backpack",               L"Рюкзак" },
        { L"Desert Banjo",                  L"Банджо" },
        { L"Desert Bird Cage",              L"Птичья клетка" },
        { L"Desert Cowboy action figure",   L"Фигурка ковбоя" },
        { L"Desert Cowboy boot trophy",     L"Трофей-сапог" },
        { L"Desert Distillery",             L"Самогонный аппарат" },
        { L"Desert Energy Crystal Chunk",   L"Кусок энергокристалла" },
        { L"Desert Explosive Tower",        L"Взрывная башня" },
        { L"Desert Fluffy Buddy",           L"Пушистый дружок" },
        { L"Desert Fossil",                 L"Окаменелость" },
        { L"Desert Gold Bar",               L"Золотой слиток" },
        { L"Desert Golden Coin",            L"Золотая монета" },
        { L"Desert Golden Horse Shoe",      L"Золотая подкова" },
        { L"Desert Golden lock",            L"Золотой замок" },
        { L"Desert Golden Shovel",          L"Золотая лопата" },
        { L"Desert Golden Wheel",           L"Золотое колесо" },
        { L"Desert Golfcart",               L"Гольф-кар" },
        { L"Desert Harmonica",              L"Губная гармошка" },
        { L"Desert Horse on wheels",        L"Лошадка на колёсах" },
        { L"Desert Lighter",                L"Зажигалка" },
        { L"Desert Lunch box",              L"Ланч-бокс" },
        { L"Desert Magnet",                 L"Магнит" },
        { L"Desert Mineral case",           L"Коллекция минералов" },
        { L"Desert Mouth Harp",             L"Варган" },
        { L"Desert Pan Flute",              L"Флейта Пана" },
        { L"Desert Popcorn stand",          L"Аппарат для попкорна" },
        { L"Desert Post card",              L"Открытка" },
        { L"Desert Rattlesnake",            L"Гремучая змея" },
        { L"Desert Rocking Chair",          L"Кресло-качалка" },
        { L"Desert Shooting Cowboy",        L"Стреляющий ковбой" },
        { L"Desert Sippy cup",              L"Поильник" },
        { L"Desert Snake in a can",         L"Змея в банке" },
        { L"Desert Snow globe",             L"Снежный шар" },
        { L"Desert Suitcase",               L"Чемодан" },
        { L"Desert Zero Grav crystal",      L"Кристалл невесомости" },

        // Headman Manor
        { L"Manor Animal Crate",            L"Ящик с животным" },
        { L"Manor Bottle",                  L"Бутылка" },
        { L"Manor Clown",                   L"Клоун" },
        { L"Manor Coffin",                  L"Гроб" },
        { L"Manor Diamond Display",         L"Витрина с бриллиантом" },
        { L"Manor Dinosaur",                L"Динозавр" },
        { L"Manor Doll",                    L"Кукла" },
        { L"Manor Emerald Bracelet",        L"Изумрудный браслет" },
        { L"Manor Frog",                    L"Лягушка" },
        { L"Manor Globe",                   L"Глобус" },
        { L"Manor Goblet",                  L"Кубок" },
        { L"Manor Golden Statue",           L"Золотая статуя" },
        { L"Manor Gramophone",              L"Граммофон" },
        { L"Manor Grandfather Clock",       L"Напольные часы" },
        { L"Manor Harp",                    L"Арфа" },
        { L"Manor Instrument",              L"Музыкальный инструмент" },
        { L"Manor Kettle",                  L"Чайник" },
        { L"Manor Magnifying Glass",        L"Лупа" },
        { L"Manor Map",                     L"Карта" },
        { L"Manor Money",                   L"Деньги" },
        { L"Manor Music Box",               L"Музыкальная шкатулка" },
        { L"Manor Ocarina",                 L"Окарина" },
        { L"Manor Old Camera",              L"Старый фотоаппарат" },
        { L"Manor Painting",                L"Картина" },
        { L"Manor Piano",                   L"Пианино" },
        { L"Manor Pocket Watch",            L"Карманные часы" },
        { L"Manor Radio",                   L"Радио" },
        { L"Manor Scream Doll",             L"Кричащая кукла" },
        { L"Manor Ship in a bottle",        L"Корабль в бутылке" },
        { L"Manor Telescope",               L"Телескоп" },
        { L"Manor Television",              L"Телевизор" },
        { L"Manor Toy Monkey",              L"Игрушечная обезьянка" },
        { L"Manor Trophy",                  L"Трофей" },
        { L"Manor Uranium Mug",             L"Урановая кружка" },
        { L"Manor Uranium Plate",           L"Урановая тарелка" },
        { L"Manor Vase",                    L"Ваза" },
        { L"Manor Vase Big",                L"Большая ваза" },
        { L"Manor Vase Chunky",             L"Массивная ваза" },
        { L"Manor Vase Small",              L"Маленькая ваза" },

        // Museum of Human Art
        { L"Museum Baby Head",              L"Голова младенца" },
        { L"Museum Banana Bow",             L"Банановый лук" },
        { L"Museum Blender",                L"Блендер" },
        { L"Museum Boombox",                L"Бумбокс" },
        { L"Museum Car",                    L"Машинка" },
        { L"Museum Cocktail",               L"Коктейль" },
        { L"Museum Cool brain",             L"Крутой мозг" },
        { L"Museum Cube ball",              L"Кубический мяч" },
        { L"Museum Cubic Sculpture",        L"Кубическая скульптура" },
        { L"Museum Cubic Tower",            L"Кубическая башня" },
        { L"Museum duck man",               L"Человек-утка" },
        { L"Museum Egg",                    L"Яйцо" },
        { L"Museum Fish",                   L"Рыба" },
        { L"Museum Flesh Blob",             L"Комок плоти" },
        { L"Museum Gem Burger",             L"Бургер с самоцветами" },
        { L"Museum Golden Swirl",           L"Золотой завиток" },
        { L"Museum GoldFish",               L"Золотая рыбка" },
        { L"Museum GoldTooth",              L"Золотой зуб" },
        { L"Museum Gumball",                L"Автомат с жвачкой" },
        { L"Museum Handface",               L"Лицо-рука" },
        { L"Museum Horse",                  L"Лошадь" },
        { L"Museum ladybug",                L"Божья коровка" },
        { L"Museum Milk",                   L"Молоко" },
        { L"Museum MonkeyBox",              L"Коробка с обезьянкой" },
        { L"Museum Pacifier",               L"Соска" },
        { L"Museum PimpleGuy",              L"Прыщавый человечек" },
        { L"Museum Plane",                  L"Самолётик" },
        { L"Museum RubenDoll",              L"Кукла Рубена" },
        { L"Museum SilverFish",             L"Серебряная рыбка" },
        { L"Museum Tall Guy",               L"Высокий парень" },
        { L"Museum Teeth Bot",              L"Зубастый робот" },
        { L"Museum Toast",                  L"Тост" },
        { L"Museum Tooth",                  L"Зуб" },
        { L"Museum Traffic Light",          L"Светофор" },
        { L"Museum Tray",                   L"Поднос" },
        { L"Museum Uranium Mug Deluxe",     L"Урановая кружка «Делюкс»" },
        { L"Museum Vinyl",                  L"Виниловая пластинка" },
        { L"Museum Wire Figure",            L"Проволочная фигурка" },
        { L"Museum Worm",                   L"Червяк" },

        // Swiftbroom Academy
        { L"Wizard Alchemy Station",        L"Алхимическая станция" },
        { L"Wizard Bird Skull",             L"Птичий череп" },
        { L"Wizard Broom",                  L"Метла" },
        { L"Wizard Bug",                    L"Жук" },
        { L"Wizard Cauldron Box",           L"Котёл" },
        { L"Wizard Chomp Book",             L"Кусачая книга" },
        { L"Wizard Crown",                  L"Корона" },
        { L"Wizard Crystal",                L"Кристалл" },
        { L"Wizard Crystal Ball",           L"Хрустальный шар" },
        { L"Wizard Cube of Knowledge",      L"Куб знаний" },
        { L"Wizard Diamond",                L"Бриллиант" },
        { L"Wizard Dragon Skull",           L"Череп дракона" },
        { L"Wizard Dumgolfs Staff",         L"Посох Дамгольфа" },
        { L"Wizard Eye Ball",               L"Глазное яблоко" },
        { L"Wizard Eye of Orpigox",         L"Око Орпигокса" },
        { L"Wizard Forever Candle",         L"Вечная свеча" },
        { L"Wizard Fortune Card",           L"Карта судьбы" },
        { L"Wizard Gem Box",                L"Шкатулка с самоцветами" },
        { L"Wizard Glowing Jar",            L"Светящаяся банка" },
        { L"Wizard Goblin Arm",             L"Рука гоблина" },
        { L"Wizard Goblin Head",            L"Голова гоблина" },
        { L"Wizard Griffin Statue",         L"Статуя грифона" },
        { L"Wizard Levitation Potion",      L"Зелье левитации" },
        { L"Wizard Love Potion",            L"Любовное зелье" },
        { L"Wizard Master Potion",          L"Великое зелье" },
        { L"Wizard Pendant",                L"Кулон" },
        { L"Wizard Poison Chalice",         L"Отравленная чаша" },
        { L"Wizard Power Crystal",          L"Кристалл силы" },
        { L"Wizard Red Mushroom",           L"Красный гриб" },
        { L"Wizard Small Gem",              L"Маленький самоцвет" },
        { L"Wizard Small Potion",           L"Маленькое зелье" },
        { L"Wizard Spider Potion",          L"Паучье зелье" },
        { L"Wizard Star Wand",              L"Звёздная палочка" },
        { L"Wizard Sword",                  L"Меч" },
        { L"Wizard Tentacle",               L"Щупальце" },
        { L"Wizard Time Glass",             L"Песочные часы" },
        { L"Wizard Troll Finger",           L"Палец тролля" },
        { L"Wizard Unicorn Horn",           L"Рог единорога" },
    };

    struct ValuableNameParts
    {
        Hax::WStringView    ThemeRu;    // empty when the name has no known theme word
        Hax::WStringView    NameRu;     // translated name, or the English one
    };

    // "Arctic Eraser" -> { "Арктика", "Ластик" }
    inline ValuableNameParts TranslateValuableName(Hax::WStringView name)
    {
        ValuableNameParts parts{ Hax::WStringView(L""), name };

        Hax::WStringView rest = name;
        for (const ValuableThemeRu& theme : g_ValuableThemesRu)
        {
            if (name.Length() > theme.En.Length() + 1 && name.StartsWith(theme.En) && name[theme.En.Length()] == L' ')
            {
                parts.ThemeRu = theme.Ru;
                rest = name.Substr(theme.En.Length() + 1);
                break;
            }
        }

        parts.NameRu = rest;
        for (const ValuableNameRu& entry : g_ValuableNamesRu)
        {
            if (entry.En == name)
            {
                parts.NameRu = entry.Ru;
                break;
            }
        }
        return parts;
    }
}
