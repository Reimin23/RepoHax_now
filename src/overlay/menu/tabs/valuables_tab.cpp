#include <src/pch.h>

#include <src/cheat.h>

#include "../localization.h"
#include "../widgets.h"

#define LINE_ID (HAX_LINE * 789)

namespace Cheat
{
    static LocKey ValuableSizeLocKey(ValuableSize size)
    {
        switch (size)
        {
        case ValuableSize::Tiny:     return LocKey_SizeTiny;
        case ValuableSize::Small:    return LocKey_SizeSmall;
        case ValuableSize::Medium:   return LocKey_SizeMedium;
        case ValuableSize::Big:      return LocKey_SizeBig;
        case ValuableSize::Wide:     return LocKey_SizeWide;
        case ValuableSize::Tall:     return LocKey_SizeTall;
        default:                     return LocKey_SizeVeryTall;
        }
    }

    // "$800", "$1.5K", "$12K"
    static void AppendMoney(Hax::WStringBuilder<192>& sb, float value)
    {
        const int v = (int)value;
        if (v < 1000)
            sb.AppendF(L"%d", v);
        else if (v % 1000 == 0)
            sb.AppendF(L"%dK", v / 1000);
        else
            sb.AppendF(L"%d.%dK", v / 1000, (v % 1000) / 100);
    }

    // "Ластик (Арктика) — крошечная, $1K–2K" / "Arctic Eraser — tiny, $1K–2K"
    static void ValuableLabel(const ValuableSpawnEntry& e, Hax::WStringBuilder<192>& sb)
    {
        if (G->Language == Lang_Ru)
        {
            sb.AppendF(L"%.*ls", (int)e.NameRu.Length(), e.NameRu.Data());
            if (!e.ThemeRu.Empty())
                sb.AppendF(L" (%.*ls)", (int)e.ThemeRu.Length(), e.ThemeRu.Data());
        }
        else
        {
            sb.AppendF(L"%.*ls", (int)e.Name.Length(), e.Name.Data());
        }

        const Hax::WStringView size = G->Loc[ValuableSizeLocKey(e.Size)];
        sb.AppendF(L" — %.*ls, $", (int)size.Length(), size.Data());

        if (!e.PriceKnown)
        {
            sb.Append(L"…");
            return;
        }

        AppendMoney(sb, e.PriceMin);
        if (e.PriceMax > e.PriceMin)
        {
            sb.Append(L"–");
            AppendMoney(sb, e.PriceMax);
        }
    }

    static void DrawValuableSpawnPanel()
    {
        BeginPanel(LINE_ID);
        PanelHeader(G->Loc[LocKey_SPAWNVALUABLES], G->Loc[LocKey_SpawnValuableDesc]);
        {
            static int s_Selected = -1; // index in G->ValuablesPool

            const bool ready = G->ValuablesPoolReady;
            const size_t count = ready ? G->ValuablesPool.Size() : 0;
            if (s_Selected >= (int)count)
                s_Selected = -1;

            const Hax::Vector<int>* order = ready ? &G->ValuablesOrder[G->Language == Lang_Ru ? Lang_Ru : Lang_Eng] : nullptr;

            const float spacing2 = 5_px;
            Hax::Vector2 sz = CalcButtonSize(G->Loc[LocKey_Spawn]);
            Hax::Gui::BeginHorizontal(spacing2);
            {
                constexpr size_t dropListId = Hax::Hash(L"ValuableSelect");
                const float dropListW = Hax::Gui::GetContentRegionAvail().X - spacing2 - sz.X;

                Hax::Gui::BeginVertical(3_px);
                {
                    Hax::WStringBuilder<192> preview;
                    if (s_Selected >= 0)
                        ValuableLabel(G->ValuablesPool[s_Selected], preview);
                    else
                        preview.Append(L"-");

                    if (DropdownBtn(LINE_ID, preview.View(), dropListW) && count > 0)
                        OpenPopup(dropListId, Hax::Gui::GetCursorPos());
                }
                Hax::Gui::EndVertical();

                const float selectableH = CalcButtonHeight();
                const float dropListH = selectableH * Hax::Max(1ULL, Hax::Min(count, 8ULL)) + 5_px * 2.f + 1.f;
                if (BeginDropList(dropListId, {dropListW, dropListH}))
                {
                    for (size_t i = 0; order != nullptr && i < order->Size(); ++i)
                    {
                        const int index = (*order)[i];
                        Hax::WStringBuilder<192> sb;
                        ValuableLabel(G->ValuablesPool[index], sb);
                        if (Selectable(LINE_ID + i * 10000, sb.View(), s_Selected == index, {.MinW = dropListW}))
                        {
                            s_Selected = index;
                            ClosePopup(dropListId);
                        }
                    }
                    EndDropList();
                }

                const bool enabled = !G->IsClient && G->IsInGame && s_Selected >= 0;
                if (Button(LINE_ID, G->Loc[LocKey_Spawn], {}, {.Enabled = enabled}))
                    G->ValuableToSpawn = s_Selected;
            }
            Hax::Gui::EndHorizontal();
        }
        EndPanel();
    }

    void DrawValuablesTab()
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
            PanelHeader(G->Loc[LocKey_VISUALS]);
            {
                {
                    ToggleEx(LINE_ID, G->ValuablesEsp, G->Loc[LocKey_DisplayThroughWalls], G->Loc[LocKey_WithinSelectedDistance]);
                }

                HorizontalLine(1_px);

                {
                    Hax::WStringBuilder<16> sb;
                    sb.AppendF(L"%dm", G->ValuablesEspRange);
                    SliderEx(LINE_ID, G->Loc[LocKey_DisplayDistance], sb.View(), &G->ValuablesEspRange, 5, 500, SliderConvertInt);
                }

                HorizontalLine(1_px);

                {
                    ToggleEx(LINE_ID, G->ValuablesChams, G->Loc[LocKey_XrayHighlight], G->Loc[LocKey_NotAllHighlighted]);
                }
            }
            EndPanel();

            BeginPanel(LINE_ID);
            PanelHeader(G->Loc[LocKey_VALUE]);
            {
                {
                    ToggleEx(LINE_ID, G->Unbreakable, G->Loc[LocKey_NoDamageValuables], G->Loc[LocKey_AvailableIfHost], {.Disabled = G->IsClient});
                }

                HorizontalLine(1_px);

                {
                    MainLabel(G->Loc[LocKey_ChangeValue]);
                    Hax::Gui::BeginHorizontal(5_px);
                    {
                        const float w = CalcWidgetEqWidth(2);
                        if (Button(LINE_ID, G->Loc[LocKey_ToZero], G->Loc[LocKey_HostOnly], {.Enabled = G->IsInGame && !G->IsClient, .MinW = w}))
                            G->SetToZero = true;
                        if (Button(LINE_ID, G->Loc[LocKey_ToMax], G->Loc[LocKey_HostOnly], {.Enabled = G->IsInGame  && !G->IsClient, .MinW = w}))
                            G->SetToMax = true;
                    }
                    Hax::Gui::EndHorizontal();
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
            PanelHeader(G->Loc[LocKey_EXTRACTION]);
            {
                {
                    ToggleEx(LINE_ID, G->ExtrPointsEsp, G->Loc[LocKey_DisplayThroughWalls]);
                }

                HorizontalLine(1_px);

                RoundDirector dir = RoundDirector::instance();
                bool disabled = !G->IsInGame || !dir || dir.extractionPointActive();
                if (Button(LINE_ID, G->Loc[LocKey_ActivateNext], {}, {.Enabled = !disabled, .MinW = Hax::Gui::GetContentRegionAvail().X}))
                    G->ActivateNextPoint = true;
            }
            EndPanel();

            DrawValuableSpawnPanel();
        }
        Hax::Gui::Dummy({0.f, 0.f});
        Hax::Gui::EndVertical();
        Hax::Gui::EndContainer();
    }

}