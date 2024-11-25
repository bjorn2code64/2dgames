#pragma once

#include "CommonControl.h"
#include "Menu.h"

namespace CommonControls {

	class ListView : public CommonControl
	{
	public:
		DWORD CreateAndShow(Window* parent, DWORD dwStyles = 0, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(CommonControl::Create(parent, WC_LISTVIEW, NULL, WS_CHILD | dwStyles));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		void SetItemText(int nItem, int nSubItem, LPCWSTR sz) {	// const version
			ListView_SetItemText(*this, nItem, nSubItem, (LPWSTR)sz);
		}

		void SetReportMode(BOOL bSortHeader, BOOL bShowSelAlways = TRUE) {
			AddWindowStyles(LVS_REPORT | (bShowSelAlways ? LVS_SHOWSELALWAYS : 0) | (bSortHeader ? 0 : LVS_NOSORTHEADER));
			ListView_SetExtendedListViewStyle(*this, LVS_EX_FULLROWSELECT);
		}

		int InsertColumn(int nColumn, LPCWSTR pszText, int nWidth) {
			LV_COLUMN column;
			ZeroMemory(&column, sizeof(column));
			column.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			column.fmt = LVCFMT_LEFT;
			column.cx = nWidth;
			column.pszText = const_cast<LPWSTR>(pszText);
			return ListView_InsertColumn(*this, nColumn, &column);
		}

		int InsertItem(int nItem, LPCWSTR pszText, LPARAM lParam) {
			LVITEM item;
			ZeroMemory(&item, sizeof(item));
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.iItem = nItem;
			item.lParam = lParam;
			item.pszText = const_cast<LPWSTR>(pszText);
			return ListView_InsertItem(*this, &item);
		}

		int InsertItem(int nItem, LPCWSTR pszText, LPARAM lParam, int imageIndex) {
			LVITEM item;
			ZeroMemory(&item, sizeof(item));
			item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
			item.iItem = nItem;
			item.lParam = lParam;
			item.iImage = imageIndex;
			item.pszText = const_cast<LPWSTR>(pszText);
			return ListView_InsertItem(*this, &item);
		}

		LPARAM GetItemData(int nItem) const {
			LVITEM lvi;
			lvi.mask = LVIF_PARAM;
			lvi.iItem = nItem;
			ListView_GetItem(*this, &lvi);
			return lvi.lParam;
		}

		int GetCurSel() const {
			return ListView_GetNextItem(*this, -1, LVNI_SELECTED);
		}

		LPARAM GetCurSelData() const {
			int sel = GetCurSel();
			return (sel >= 0) ? GetItemData(sel) : 0;
		}

		size_t GetCurMultiSel(std::vector<int>& sel) {
			int iPos = ListView_GetNextItem(*this, -1, LVNI_SELECTED);
			while (iPos != -1) {
				sel.push_back(iPos);
				iPos = ListView_GetNextItem(*this, iPos, LVNI_SELECTED);
			}
			return sel.size();
		}

		size_t GetCurMultiSelData(std::vector<LPARAM>& sel) {
			int iPos = ListView_GetNextItem(*this, -1, LVNI_SELECTED);
			while (iPos != -1) {
				sel.push_back(GetItemData(iPos));
				iPos = ListView_GetNextItem(*this, iPos, LVNI_SELECTED);
			}
			return sel.size();
		}

		void SetCurSelByData(LPARAM lParam) {
			ListView_SetItemState(*this, -1, 0, LVIS_SELECTED | LVIS_FOCUSED);	// unselect
			int n = ListView_GetItemCount(*this);
			for (int i = 0; i < n; i++) {
				if (GetItemData(i) == lParam) {
					ListView_SetItemState(*this, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
			}
		}

		void SetCurSel(int nIndex) {
			ListView_SetItemState(*this, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}

		void ClearSelection() {
			ListView_SetItemState(*this, -1, LVIF_STATE, LVIS_SELECTED);
		}

		int GetIndexByData(LPARAM lParam) {
			int n = ListView_GetItemCount(*this);
			for (int i = 0; i < n; i++) {
				if (GetItemData(i) == lParam) {
					return i;
				}
			}
			return -1;
		}

		int GetColumnCount() {
			return Header_GetItemCount(ListView_GetHeader(*this));
		}

		void DeleteAllColumns() {
			LVCOLUMN col;
			col.mask = LVCF_WIDTH;
			while (ListView_GetColumn(*this, 0, &col))
				ListView_DeleteColumn(*this, 0);
		}

		void VirtualSetRowCount(int count, bool refreshItems) {
			::SendMessage(*this, LVM_SETITEMCOUNT, count, (refreshItems ? 0 : LVSICF_NOINVALIDATEALL) | LVSICF_NOSCROLL);
		}
	};

	class HeaderControl : public CommonControl
	{
	public:
		HeaderControl(const ListView& lv) {
			SetHWnd(ListView_GetHeader(lv));
		}
		~HeaderControl() {
		}

		int HitTest(const POINT& pt) {
			w32Rect r;
			for (int i = 0; i < Header_GetItemCount(*this); i++) {
				Header_GetItemRect(*this, i, &r);
				if (r.ptInRect(pt)) {
					return i;
				}
			}
			return -1;
		}
	};
}
