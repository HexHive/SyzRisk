static int  setup_swap_account(char *s)
{
	pr_warn_once("The swapaccount= commandline option is deprecated. "
		     "Please report your usecase to linux-mm@kvack.org if you "
		     "depend on this functionality.\n");
	return 1;
}

struct setup_swap_account__HEXSHA { void _b25806dcd3d5; };
struct setup_swap_account__ATTRS { void ___init; };
