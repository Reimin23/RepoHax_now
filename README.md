# RepoHax — updated for R.E.P.O. v0.5.2 (unofficial)

> 🇷🇺 **Мод обновлён под новую версию игры R.E.P.O. v0.5.2.**
> Это неофициальное обновление мода [RepoHax](https://github.com/Sacracia/RepoHax) от **Sacracia**.
> Все права на оригинальный мод принадлежат его автору (лицензия MIT).

> 🇬🇧 **The mod is updated for the new game version, R.E.P.O. v0.5.2.**
> This is an unofficial update of [RepoHax](https://github.com/Sacracia/RepoHax) by **Sacracia**.
> All credit for the original mod goes to its author (MIT license).

## Установка / Installation

1. Скачайте `dwmapi.dll` из папки [`Build`](Build).
   Download `dwmapi.dll` from the [`Build`](Build) folder.
2. Положите его рядом с `REPO.exe` (Steam → R.E.P.O. → Управление → Просмотреть локальные файлы).
   Put it next to `REPO.exe` (Steam → R.E.P.O. → Manage → Browse local files).
3. Запустите игру. Меню открывается клавишей `~`.
   Start the game. Open the menu with `~`.

`dwmapi.pdb` необязателен: если положить его рядом с dll, в отчётах о крашах будут имена функций.
`dwmapi.pdb` is optional: next to the dll it adds function names to crash reports.

## Сборка из исходников / Building from source

Нужна Visual Studio 2022 с компонентом «Разработка классических приложений на C++».
Requires Visual Studio 2022 with the "Desktop development with C++" workload.

- Откройте `RepoHax.sln`, выберите `Release` / `x64` и соберите проект, или запустите `build.ps1` в PowerShell.
  Open `RepoHax.sln`, select `Release` / `x64` and build, or run `build.ps1` in PowerShell.
- Готовый файл: `build\Release\bin\dwmapi.dll`.
  Output: `build\Release\bin\dwmapi.dll`.

Папку `Build` после каждого изменения в `main` автоматически обновляет GitHub Actions.
The `Build` folder is rebuilt automatically by GitHub Actions on every push to `main`.

## Что изменилось / What's new

**Обновление под v0.5.1 / Game update**
- Мод снова работает на R.E.P.O. v0.5.2 (Unity 6): исправлены вызовы движка, изменившиеся методы и поля игры.
  Works again on R.E.P.O. v0.5.2 (Unity 6): fixed engine calls and changed game methods and fields.
- Исправлен краш при включении ESP (отображения через стены).
  Fixed the crash when ESP (display through walls) is enabled.

**Исправления / Fixes**
- «Максимальная батарея» снова работает: оружие больше не разряжается.
  "Max battery" works again: guns no longer run out of ammo.
- «Неограниченная дальность» теперь работает и при отдалении предмета колёсиком мыши.
  "Unlimited range" now also works when pushing a held object away with the mouse wheel.

**Новое / New**
- Спавн ценностей: все 192 ценности со всех уровней, с размером и диапазоном цены.
  Valuable spawning: all 192 valuables from every level, with size and price range.
- Все предметы и ценности в списках спавна переведены на русский.
  All items and valuables in the spawn lists are translated into Russian.
- Отчёты о крашах: если игра упадёт, в папке `haxsdk_crashes` рядом с игрой появится файл с причиной.
  Crash reports: if the game crashes, a report appears in the `haxsdk_crashes` folder next to the game.

## Важно / Notes

- Спавн предметов и ценностей, батарея и часть других функций работают только у хоста лобби.
  Spawning, max battery and some other features work only for the lobby host.
- Используйте на свой риск, лучше в одиночной игре или с друзьями.
  Use at your own risk, preferably in singleplayer or with friends.

## Авторы / Credits

- Оригинальный мод / Original mod: [Sacracia/RepoHax](https://github.com/Sacracia/RepoHax)
- Обновление под v0.5.1 / v0.5.1 update: Reimin23

---

# Оригинальный README / Original README

<img width="1338" height="691" alt="Screenshot 2026-04-19 021713 (1)" src="https://github.com/user-attachments/assets/aa2fec40-b32f-4616-b4df-26789feb9843" />

### About
**RepoHax** is an advanced internal C++ cheat for the game **REPO**, developed with minimal reliance on third-party libraries. The project features custom graphics library and SDK for interacting with the Unity engine directly from C++ (both will be released as standalone open-source projects in the future).

> [!IMPORTANT]
> This project was created for educational purposes as part of a university project. It aims to provide developers with deeper insights into game hacking / mod development aspects.

---

### Official Sources
> [!CAUTION]
> To ensure your security, only download the mod from these official sources:
> 1. [This GitHub Repository] https://github.com/Sacracia/RepoHax
> 2. [Nexus Mods] https://www.nexusmods.com/repo/mods/214
> 3. [Playground] https://users.playground.ru/3241576/posts

---

### Features

**Health**
- God Mode
- Heal to Maximum

**Movement**
- Infinite Stamina
- Modify Walk / Crouch / Sprint Speed
- Infinite Jumps
- No tumble (fall only by pressing hotkey)

**Grabbing**
- High Strength (Host / Single-player only)
- Unlimited Grab Range
- No Overload

**Upgrades**
- Apply purchased upgrades (or shop upgrades) to self

**Medkits**
- Apply purchased medkit to self

**Enemies**
- Spawn (Host / Single-player only)
- Blind (Host / Single-player only, enemies can still hear)
- No Hold Time Limit (Host / Single-player only)
- Display through walls (ESP)

**Players**
- Display through walls (ESP)
- Chams (Highlight through walls)
- Tumble selected player
- Kill / Revive selected player (Host / Single-player only)
- Toggle Voice Chat: Alive / Dead

**Vision**
- Enhanced Vision
- Third Person
- Field of View (FOV)
- Infinite Dead Head Charge

**Flashlight**
- Light Intensity
- Lighting Angle
- Works while crawling

**Valuables**
- Display through walls (ESP)
- Chams (Highlight through walls)
- No Damage (Host / Single-player only)
- Set all prices to Zero / Maximum

**Collection Points**
- Display through walls (ESP)
- Activate next point

**Weapons**
- Maximum Battery while holding
- Laser Sight

**Items**
- Spawn (Host / Single-player only)

**Level**
- Modify completed levels count (Host / Single-player only)
- Skip to next level (Host / Single-player only)

**Map Filter**
- Remove maps from rotation (Host / Single-player only)

**Truck**
- Display through walls (ESP)
- Teleport to truck

---

### Configuration
The menu hotkey, interface language, and menu options can be adjusted in the **Settings** tab and are saved to `haxsdk.ini`.

---

### Installation

For convenience and to avoid using external injectors, this mod utilizes **DLL Proxying**:

1. Download the `version.dll` file.
2. Place it in the game's root directory (the folder containing the `.exe` file).
3. Launch the game. The mod will load automatically and the menu will appear.

*If you prefer using a **DLL Injector**, simply rename dll file to something else and proceed with your preferred injector.*

---

## ❗ Important Note
Using external mod loaders (such as **BepInEx** or **MelonLoader**) may cause conflicts. RepoHax uses its own hooking system, which may be overwritten by other tools. It is recommended to use this on a clean game installation.
