/* Generated automatically from 'metatheme.h'. */

package metatheme;

public interface MT {

    /* widget types: */

    public static final int MT_NONE = 0;
    public static final int MT_BASE = 1;
    public static final int MT_WINDOW_BASE = 2;
    public static final int MT_BORDER_IN = 3;
    public static final int MT_BORDER_OUT = 4;
    public static final int MT_HLINE = 5;
    public static final int MT_VLINE = 6;
    public static final int MT_FOCUS_TAB = 7;
    public static final int MT_FOCUS_CHECKBOX = 8;
    public static final int MT_RESIZE_GRIP = 9;
    public static final int MT_ARROW_UP = 10;
    public static final int MT_ARROW_DOWN = 11;
    public static final int MT_ARROW_LEFT = 12;
    public static final int MT_ARROW_RIGHT = 13;
    public static final int MT_BUTTON = 14;
    public static final int MT_BUTTON_DEFAULT = 15;
    public static final int MT_BUTTON_TOGGLE = 16;
    public static final int MT_BUTTON_HEADER = 17;
    public static final int MT_MENU = 18;
    public static final int MT_MENU_ITEM = 19;
    public static final int MT_MENU_ITEM_CHECK = 20;
    public static final int MT_MENU_ITEM_RADIO = 21;
    public static final int MT_MENU_ITEM_ARROW = 22;
    public static final int MT_MENU_ITEM_SEPARATOR = 23;
    public static final int MT_MENU_TEAROFF = 24;
    public static final int MT_MENUBAR = 25;
    public static final int MT_MENUBAR_ITEM = 26;
    public static final int MT_TOOLBAR = 27;
    public static final int MT_TOOLBAR_ITEM = 28;
    public static final int MT_TOOLBAR_ITEM_TOGGLE = 29;
    public static final int MT_TOOLBAR_ITEM_DROPDOWN = 30;
    public static final int MT_TOOLBAR_SEPARATOR = 31;
    public static final int MT_TOOLBAR_HANDLE = 32;
    public static final int MT_SCROLLBAR = 33;
    public static final int MT_SCROLLBAR_HANDLE = 34;
    public static final int MT_SCROLLBAR_ARROW_UP = 35;
    public static final int MT_SCROLLBAR_ARROW_DOWN = 36;
    public static final int MT_SCROLLBAR_ARROW_LEFT = 37;
    public static final int MT_SCROLLBAR_ARROW_RIGHT = 38;
    public static final int MT_PROGRESSBAR = 39;
    public static final int MT_PROGRESSBAR_SLIDER = 40;
    public static final int MT_NOTEBOOK = 41;
    public static final int MT_NOTEBOOK_TAB = 42;
    public static final int MT_NOTEBOOK_ARROW_LEFT = 43;
    public static final int MT_NOTEBOOK_ARROW_RIGHT = 44;
    public static final int MT_RADIO_BUTTON = 45;
    public static final int MT_CHECK_BUTTON = 46;
    public static final int MT_CHECK_BOX = 47;
    public static final int MT_RADIO_BOX = 48;
    public static final int MT_CHECK_CELL = 49;
    public static final int MT_RADIO_CELL = 50;
    public static final int MT_CHOICE = 51;
    public static final int MT_CHOICE_BUTTON = 52;
    public static final int MT_SCALE = 53;
    public static final int MT_SCALE_HANDLE = 54;
    public static final int MT_STATUSBAR_SECTION = 55;
    public static final int MT_STATICBOX = 56;
    public static final int MT_SPINBUTTON = 57;
    public static final int MT_SPINBUTTON_UP = 58;
    public static final int MT_SPINBUTTON_DOWN = 59;
    public static final int MT_ENTRY = 60;
    public static final int MT_ENTRY_BORDER = 61;
    public static final int MT_SCROLLED_WINDOW = 62;
    public static final int MT_TOOLTIP = 63;
    public static final int MT_SPLITTER = 64;

    /* state types: */

    public static final int MT_NORMAL = 0x0000;
    public static final int MT_HOVER = 0x0001;
    public static final int MT_ACTIVE = 0x0002;
    public static final int MT_DISABLED = 0x0004;
    public static final int MT_SELECTED = 0x0008;
    public static final int MT_MOUSE_ACTIVE = 0x0010;

    /* metric values: */

    public static final int MT_BORDER = 0;
    public static final int MT_SCROLLBAR_SIZE = 1;
    public static final int MT_CHECKBOX_SIZE = 2;
    public static final int MT_SCALE_SLIDER_SIZE = 3;
    public static final int MT_STATICBOX_TEXT_OFFSET = 4;
    public static final int MT_MENUBAR_ITEM_SPACING = 5;
    public static final int MT_MENU_SEPARATOR_HEIGHT = 6;
    public static final int MT_NOTEBOOK_OVERLAP = 7;
    public static final int MT_NOTEBOOK_TAB_OVERLAP = 8;
    public static final int MT_NOTEBOOK_TEXT_OFFSET = 9;
    public static final int MT_NOTEBOOK_ARROW_WIDTH = 10;
    public static final int MT_NOTEBOOK_IS_FILLED = 11;
    public static final int MT_CHOICE_BUTTON_WIDTH = 12;
    public static final int MT_TOOLBAR_ITEM_DROPDOWN_WIDTH = 13;
    public static final int MT_SPLITTER_WIDTH = 14;
    public static final int MT_MENU_TEAROFF_HEIGHT = 15;
    public static final int MT_DETACHED_MENU_TEAROFF_HEIGHT = 16;
    public static final int MT_METRIC_MAX = 17;

    /* metric size values: */

    public static final int MT_BUTTON_BORDER = 0;
    public static final int MT_BUTTON_TEXT_OFFSET = 1;
    public static final int MT_BUTTON_TOGGLE_BORDER = 2;
    public static final int MT_BUTTON_TOGGLE_TEXT_OFFSET = 3;
    public static final int MT_TOOLBAR_ITEM_TEXT_OFFSET = 4;
    public static final int MT_TOOLBAR_ITEM_BORDER = 5;
    public static final int MT_MENUBAR_BORDER = 6;
    public static final int MT_MENUBAR_ITEM_BORDER = 7;
    public static final int MT_MENUBAR_ITEM_TEXT_OFFSET = 8;
    public static final int MT_MENU_BORDER = 9;
    public static final int MT_MENU_ITEM_BORDER = 10;
    public static final int MT_PROGRESSBAR_BORDER = 11;
    public static final int MT_METRIC_SIZE_MAX = 12;

    /* color types: */

    public static final int MT_BACKGROUND = 0;
    public static final int MT_FOREGROUND = 1;
    public static final int MT_WINDOW_BACKGROUND = 2;
    public static final int MT_WINDOW_FOREGROUND = 3;
    public static final int MT_BUTTON_BACKGROUND = 4;
    public static final int MT_BUTTON_FOREGROUND = 5;
    public static final int MT_SELECTED_BACKGROUND = 6;
    public static final int MT_SELECTED_FOREGROUND = 7;
    public static final int MT_DISABLED_BACKGROUND = 8;
    public static final int MT_DISABLED_FOREGROUND = 9;
    public static final int MT_BUTTON_LIGHT = 10;
    public static final int MT_BUTTON_MIDLIGHT = 11;
    public static final int MT_BUTTON_DARK = 12;
    public static final int MT_BUTTON_MID = 13;
    public static final int MT_BUTTON_SHADOW = 14;
    public static final int MT_COLOR_MAX = 15;

    /* position types: */

    public static final int MT_POSITION_LEFT = 0;
    public static final int MT_POSITION_RIGHT = 1;
    public static final int MT_POSITION_TOP = 2;
    public static final int MT_POSITION_BOTTOM = 3;

    /* orientation types: */

    public static final int MT_HORIZONTAL = 0;
    public static final int MT_VERTICAL = 1;

    /* widget flags: */

    public static final int MT_SCROLLBAR_UNSCROLLABLE = 0x0001;
    public static final int MT_SCROLLBAR_SUBPAGE_ACTIVE = 0x0002;
    public static final int MT_SCROLLBAR_ADDPAGE_ACTIVE = 0x0004;
    public static final int MT_NOTEBOOK_FIRST_VISIBLE_TAB = 0x0001;
    public static final int MT_NOTEBOOK_FIRST_TAB = 0x0002;
    public static final int MT_NOTEBOOK_LAST_TAB = 0x0004;
    public static final int MT_SPINBUTTON_HOVER = 0x0001;
    public static final int MT_TOOLBAR_ITEM_DROPDOWN_ACTIVE = 0x0001;
    public static final int MT_MENU_DETACHED = 0x0001;
    public static final int MT_DRAW_MASK = 0x0001;
    public static final int MT_PROGRESSBAR_REVERSED = 0x0001;

};