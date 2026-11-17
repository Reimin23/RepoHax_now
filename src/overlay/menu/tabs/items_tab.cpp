#include <src/pch.h>

#include <src/cheat.h>

#include "../localization.h"
#include "../item_names.h"
#include "../widgets.h"

#include <algorithm>

#define LINE_ID (HAX_LINE * 234)

namespace Cheat
{
    // Indices into G->ItemsPool sorted by the name shown in the current language.
    static const Hax::Vector<size_t>& GetSortedItemOrder()
    {
        static Hax::Vector<size_t> s_Order;
        static size_t s_Count = (size_t)-1;
        static int s_Lang = -1;

        const size_t count = G->ItemsPool.Size();
        if (count != s_Count || G->Language != s_Lang)
        {
            s_Count = count;
            s_Lang = G->Language;

            s_Order.Clear();
            s_Order.Reserve(count);
            for (size_t i = 0; i < count; ++i)
                s_Order.PushBack(i);

            std::sort(s_Order.begin(), s_Order.end(), [](size_t a, size_t b)
            {
                return MenuNameLess(LocalizeItemName(G->ItemsPool.begin()[a].key, G->Language),
                                    LocalizeItemName(G->ItemsPool.begin()[b].key, G->Language));
            });
        }
        return s_Order;
    }

    void DrawItemsTab()
    {
        const Hax::Vector2 mainAreaSize = Hax::Gui::GetContentRegionAvail();
        const float spacing = 20_px;
        const Hax::Vector2 columnSize = {(mainAreaSize.X - 3 * spacing) / 2, mainAreaSize.Y};

        // Column 1
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(0, {.W = columnSize.X, .H = columnSize.Y});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_GUNS]);
            {
                {
                    bool enabled = !G->IsClient;
                    ToggleEx(LINE_ID, G->InfBattery, G->Loc[LocKey_MaxBatteryGun], G->Loc[LocKey_HostOnly], {.Disabled = !enabled});
                }

                HorizontalLine(1_px);

                {
                    ToggleEx(LINE_ID, G->UseLaser, G->Loc[LocKey_LaserCrosshair]);
                }
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();

        // Column 2
        Hax::Gui::Space(spacing);
        Hax::Gui::BeginContainer(0, {.W = columnSize.X, .H = columnSize.Y});
        Hax::Gui::BeginVertical(spacing);
        Hax::Gui::Dummy({0.f, 0.f});
        {
            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_SPAWN], G->Loc[LocKey_AvailableIfHost]);
            {
                static Item s_SelectedItem;

                const float spacing2 = 5_px;
                Hax::Vector2 sz = CalcButtonSize(G->Loc[LocKey_Spawn]);
                Hax::Gui::BeginHorizontal(spacing2);
                {
                    size_t nItems = G->ItemsPool.Size();
                    constexpr size_t dropListId = Hax::Hash(L"ItemSelect");
                    const float dropListW = Hax::Gui::GetContentRegionAvail().X - spacing2 - sz.X;

                    Hax::Gui::BeginVertical(3_px);
                    {
                        Hax::WStringView preview = L"-";
                        if (s_SelectedItem != null)
                            preview = LocalizeItemName(s_SelectedItem.itemName().ToHaxView(), G->Language);

                        if (DropdownBtn(LINE_ID, preview, dropListW))
                            OpenPopup(dropListId, Hax::Gui::GetCursorPos());
                    }
                    Hax::Gui::EndVertical();

                    const float selectableH = CalcButtonHeight();
                    const float dropListH = selectableH * Hax::Max(1ULL, Hax::Min(nItems, 8ULL)) + 5_px * 2.f + 1.f;
                    if (BeginDropList(dropListId, {dropListW, dropListH}))
                    {
                        const Hax::Vector<size_t>& order = GetSortedItemOrder();
                        for (size_t i = 0; i < nItems && i < order.Size(); ++i)
                        {
                            const auto& p = G->ItemsPool.begin()[order[i]];
                            if (Selectable(LINE_ID + i * 10000, LocalizeItemName(p.key, G->Language), s_SelectedItem == p.value, {.MinW = dropListW}))
                            {
                                s_SelectedItem = p.value;
                                ClosePopup(dropListId);
                            }
                        }
                        EndDropList();
                    }

                    bool enabled = !G->IsClient && s_SelectedItem != null && G->IsInGame;
                    if (Button(LINE_ID, G->Loc[LocKey_Spawn], {}, {.Enabled = enabled}))
                        G->ItemToSpawn = s_SelectedItem;
                }
                Hax::Gui::EndHorizontal();
            }
            EndPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }
}