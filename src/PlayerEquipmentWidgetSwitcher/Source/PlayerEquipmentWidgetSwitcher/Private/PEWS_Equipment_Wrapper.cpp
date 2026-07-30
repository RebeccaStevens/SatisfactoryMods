#include "PEWS_Equipment_Wrapper.h"
#include "PEWS_Panel.h"

void UPEWS_Equipment_Wrapper::NativeConstruct() {
	Super::NativeConstruct();
	ResetNavigationHistory();
}

int32 UPEWS_Equipment_Wrapper::AddChild(UWidget* Widget) {
	const auto Switcher = GetWidgetSwitcher();
	Switcher->AddChild(Widget);
	return Switcher->GetNumWidgets() - 1;
}

void UPEWS_Equipment_Wrapper::ResetNavigationHistory(const bool PerformNavigation) {
	NavigationHistory.Empty();
	NavigationHistory.Push(DefaultWidgetHistoryEntry);
	NavigationForwardHistory.Empty();

	if (PerformNavigation) {
		SetActiveChild(DefaultWidgetHistoryEntry);
	}
}

void UPEWS_Equipment_Wrapper::NavigateTo(const FPEWS_NavigationHistoryEntry& Entry) {
	NavigationHistory.Push(Entry);
	NavigationForwardHistory.Empty();
	SetActiveChild(Entry);
}

void UPEWS_Equipment_Wrapper::NavigateTo(const int32 Index, UPEWS_NavigationHistoryEntryData* Data) {
	NavigateTo({ Index, Data });
}

bool UPEWS_Equipment_Wrapper::NavigateBack() {
	if (NavigationHistory.Num() <= 1) {
		return false;
	}
	NavigationHistory.Pop();
	const auto& Entry = NavigationHistory.Last();
	NavigationForwardHistory.Push(Entry);
	SetActiveChild(Entry);
	return true;
}

bool UPEWS_Equipment_Wrapper::NavigateForward() {
	if (NavigationForwardHistory.IsEmpty()) {
		return false;
	}
	const auto Entry = NavigationForwardHistory.Pop();
	NavigationHistory.Push(Entry);
	SetActiveChild(Entry);
	return true;
}

void UPEWS_Equipment_Wrapper::SetActiveChild(const FPEWS_NavigationHistoryEntry& Entry) {
	const auto Switcher = GetWidgetSwitcher();
	check(IsValid(Switcher));
	const auto NumWidgets = Switcher->GetNumWidgets();

	if (!ensureMsgf(Entry.Index >= 0 && Entry.Index < NumWidgets, TEXT("Invalid NavigationHistoryEntry index (%d)"), NumWidgets)) {
		return;
	}

	Switcher->SetActiveWidgetIndex(Entry.Index);
	const auto Active = Switcher->GetActiveWidget();
	check(IsValid(Active));
	if (Active->GetClass()->ImplementsInterface(UPEWS_Panel::StaticClass())) {
		IPEWS_Panel::Execute_OnNavigate(Active, Entry.Data, GetDesiredChildSize(Active));
	}
}
