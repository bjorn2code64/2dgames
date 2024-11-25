#pragma once

#include "CommonControl.h"

namespace CommonControls {

	class TreeView : public CommonControl
	{
	public:
		DWORD CreateAndShow(Window* parent, DWORD dwStyles, int nCmdShow = SW_SHOW) {
			RETURN_IF_ERROR(CommonControl::Create(parent, WC_TREEVIEW, NULL, WS_CHILD | dwStyles));

			::ShowWindow(*this, nCmdShow);
			return ERROR_SUCCESS;
		}

		HTREEITEM InsertItem(LPCTSTR pszText, HTREEITEM hItemParent = TVI_ROOT, LPARAM lParam = 0) {
			TVINSERTSTRUCT item;
			ZeroMemory(&item, sizeof(item));
			item.hParent = hItemParent;
			item.hInsertAfter = TVI_LAST;
			item.item.mask = TVIF_TEXT | TVIF_PARAM;
			item.item.pszText = const_cast<LPTSTR>(pszText);
			item.item.lParam = lParam;
			return TreeView_InsertItem(*this, &item);
		}

		LPARAM GetItemData(HTREEITEM hItem) {
			TVITEM item;
			ZeroMemory(&item, sizeof(item));
			item.mask = TVIF_PARAM;
			item.hItem = hItem;
			TreeView_GetItem(*this, &item);
			return item.lParam;
		}

		void SetCurSelByData(LPARAM lParam) {
			// A recursive search - not efficient for a big tree
			SetCurSelByData(lParam, TreeView_GetRoot(*this));
		}

		LPARAM GetCurSelItemData(LPARAM lpDefault) {
			HTREEITEM h = TreeView_GetSelection(*this);
			return h ? GetItemData(h) : lpDefault;
		}

	protected:
		BOOL SetCurSelByData(LPARAM lParam, HTREEITEM hItem) {
			if (hItem == NULL)	return FALSE;

			while (hItem) {
				if (lParam == GetItemData(hItem)) {
					TreeView_Select(*this, hItem, TVGN_CARET);
					return TRUE;
				}

				if (SetCurSelByData(lParam, TreeView_GetChild(*this, hItem)))
					return TRUE;	// Found it
				hItem = TreeView_GetNextItem(*this, hItem, TVGN_NEXT);
			}

			return FALSE;
		}
	};

}