#ifndef BADGE_MENU_H
#define	BADGE_MENU_H
#define RED_BG 0
#define GREEN_BG 0
#define BLUE_BG 0

/** Bit indexes into menu attributes variable */
enum attrib_bits {
    /* max 16 bits in unsigned short */
    VERT_BIT = 9,
    HORIZ_BIT,
    SKIP_BIT,    /**< skip when scrolling */
    DEFAULT_BIT,
    HIDDEN_BIT,  /**< Don't display this menu item */
    LAST_BIT,
};

#define VERT_ITEM    (1 << VERT_BIT)
#define HORIZ_ITEM   (1 << HORIZ_BIT)
#define SKIP_ITEM    (1 << SKIP_BIT)
#define DEFAULT_ITEM (1 << DEFAULT_BIT)
#define HIDDEN_ITEM  (1 << HIDDEN_BIT)
#define LAST_ITEM    (1 << LAST_BIT)

/** Enum of types of menu item. */
enum type {
    MORE = 0, /**< if the menu is too long to fit */
    TEXT, /**< text to display */
    BACK, /**< return to previous menu */
    MENU, /**< sub menu type */
    FUNCTION, /**< c function */
    TASK, // FreeRTOS Task
	SMENU // Special case for schedule movement
};

/** Enum to specify a menu display style. */
typedef enum {
    MAIN_MENU_STYLE,
    MAIN_MENU_WITH_TIME_DATE_STYLE,
    DRBOB_MENU_STYLE,
    WHITE_ON_BLACK,
    BLANK
} MENU_STYLE;

typedef void (*menu_func)(void*);

union menu_data_t {
    const struct menu_t *menu;
    //void (*func)(struct menu_t *m); /**< An application callback function */
    void (*func)();
    void (*task)(void *p_arg);
    void *generic;
};

/** A menu item */
struct menu_t {
    char name[16]; /**< menu item name / displayed text */
    unsigned short attrib; /**< set of attribute bits for menu item */
    unsigned char type; /**< Type of menu item (see enum) */

    /** @note when initing the union, coerce non void data to a menu_t to
        keep compiler from whining. */
    union menu_data_t data; /**< extra data used to process menu item; depends on type */
};

/**
 * Get current menu item (i.e. cursor position) for menu at stack
 * level == `item`. (Where 0 is top of stack, 1 is next level, etc.)
 */
struct menu_t *getMenuStack(unsigned char item);
/**
 * Get currently selected menu item (i.e. current menu when button was
 * pressed) for menu at stack level == `item`. (Where 0 is top of
 * stack, 1 is next level, etc.)
 */
struct menu_t *getSelectedMenuStack(unsigned char item);

/**
 * Animation for main menu (currently unimplemented).
 */
void main_menu_animator(void);

/**
 * Display new menu to LCD, using display style `style`, with item
 * `selected` highlighted as current selection.
 *
 * @param menu The menu to display.
 * @param selected The currently selected menu item.
 * @param style The menu display style to use.
 * @return The currently selected item (may be different from `selected` if that menu
 *         item was not displayed for some reason).
 */
struct menu_t *display_menu(struct menu_t *menu, struct menu_t *selected, MENU_STYLE style);
/** Clears currently running application, displays menu from top of stack. */
void returnToMenus();
/**
 * Main menu routine. Displays menu, handles button press and touch
 * sensor slides.
 */
void menus();

/** @return Currently displayed menu */
struct menu_t *getCurrMenu();
/** @return Menu item the cursor is on */
struct menu_t *getSelectedMenu();

/** @return Currently running application, or NULL */
extern void (*runningApp)();

extern unsigned int buttonTimestamp[];
/**
 * Handler for application (i.e. temporary) menus.
 *
 * @param L_menu Pointer to the application menu structure.
 * @param style Display style for the menu.
 */
void genericMenu(struct menu_t *L_menu, MENU_STYLE style);

/**
 * Pop current menu from stack, clear currently running application,
 * display previous menu.
 */
void closeMenuAndReturn();

/**
 * Displays main navigation menu and manages the launching of
 * applications in the main menu
 */
void menu_and_manage_task(void* p_arg);

#endif	/* BADGE_MENU_H */
