
// Test library configuration for gtk.cfg
//
// Usage:
// $ cppcheck --check-library --enable=information --inconclusive --error-exitcode=1 --suppress=missingIncludeSystem --inline-suppr --library=gtk test/cfg/gtk.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <gtk/gtk.h>
#include <glib.h>


void validCode(int argInt)
{
    // if G_UNLIKELY is not defined this results in a syntax error
    if G_UNLIKELY(argInt == 1) {
    } else if (G_UNLIKELY(argInt == 2)) {
    }

    if G_LIKELY(argInt == 0) {
    } else if (G_LIKELY(argInt == -1)) {
    }

    printf("%s", _("test"));
    printf("%s", Q_("a|test"));
    printf("%s", N_("test"));

    gpointer gpt = g_malloc(4);
    printf("%p", gpt);
    g_free(gpt);
    g_assert(gpt);
    if (!gpt) {
        // cppcheck-suppress checkLibraryNoReturn
        g_assert_not_reached();
    }
    gpointer p = GINT_TO_POINTER(1);
    int i = GPOINTER_TO_INT(p);
    // cppcheck-suppress knownConditionTrueFalse
    if (i == 1) {}

    g_print("test");
    g_print("%d", 1);
    g_printerr("err");

    GString * pGStr1 = g_string_new("test");
    g_string_append(pGStr1, "a");
    g_string_free(pGStr1, TRUE);

    gchar * pGchar1 = g_strconcat("a", "b", NULL);
    printf("%s", pGchar1);
    g_free(pGchar1);

    GError * pGerror = g_error_new(1, -2, "a %d", 1);
    g_error_free(pGerror);
}

void g_malloc_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_malloc(8);

    gpointer gpt = g_malloc(1);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_malloc0_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_malloc0(8);

    gpointer gpt = g_malloc0(1);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_malloc_n_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_malloc_n(8, 1);

    gpointer gpt = g_malloc_n(1, 2);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_malloc0_n_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_malloc0_n(8, 1);

    gpointer gpt = g_malloc0_n(1, 2);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_malloc_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_try_malloc(8);

    gpointer gpt = g_try_malloc(1);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_malloc0_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_try_malloc0(8);

    gpointer gpt = g_try_malloc0(1);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_malloc_n_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_try_malloc_n(8, 1);

    gpointer gpt = g_try_malloc_n(1, 2);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_malloc0_n_test()
{
    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_try_malloc0_n(8, 1);

    gpointer gpt = g_try_malloc0_n(1, 2);

    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_realloc_test()
{
    // cppcheck-suppress ignoredReturnValue
    // TODO cppcheck-suppress leakReturnValNotUsed
    g_realloc(NULL, 1);

    gpointer gpt = g_malloc(1);
    gpt = g_realloc(gpt, 2); // No memleakOnRealloc since g_realloc aborts if it fails
    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_realloc_n_test()
{
    // cppcheck-suppress ignoredReturnValue
    // TODO cppcheck-suppress leakReturnValNotUsed
    g_realloc_n(NULL, 1, 2);

    gpointer gpt = g_malloc_n(1, 2);
    gpt = g_realloc_n(gpt, 2, 3); // No memleakOnRealloc since g_realloc_n aborts if it fails
    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_realloc_test()
{
    // cppcheck-suppress ignoredReturnValue
    // TODO cppcheck-suppress leakReturnValNotUsed
    g_try_realloc(NULL, 1);

    gpointer gpt = g_try_malloc(1);
    // cppcheck-suppress memleakOnRealloc
    gpt = g_try_realloc(gpt, 2);
    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_try_realloc_n_test()
{
    // cppcheck-suppress ignoredReturnValue
    // TODO cppcheck-suppress leakReturnValNotUsed
    g_try_realloc_n(NULL, 1, 2);

    gpointer gpt = g_try_malloc_n(1, 2);
    // TODO cppcheck-suppress memleakOnRealloc
    gpt = g_try_realloc_n(gpt, 2, 3);
    printf("%p", gpt);

    // cppcheck-suppress memleak
}

void g_assert_test()
{
    int a;
    // cppcheck-suppress checkLibraryNoReturn
    // cppcheck-suppress assignmentInAssert
    g_assert(a = 5);
}

void g_print_test()
{
    // cppcheck-suppress invalidPrintfArgType_uint
    g_print("%u", -1);
    // cppcheck-suppress invalidPrintfArgType_uint
    g_printerr("%x", "a");
}

void g_alloca_test()
{
    // cppcheck-suppress allocaCalled
    char * pBuf1 = g_alloca(5);
    pBuf1[0] = '\0';
}

void g_new_test()
{
    struct a {
        int b;
    };
    // valid
    struct a * pNew1 = g_new(struct a, 5);
    printf("%p", pNew1);
    g_free(pNew1);

    // cppcheck-suppress leakReturnValNotUsed
    g_new(struct a, 1);

    struct a * pNew2 = g_new(struct a, 2);
    printf("%p", pNew2);
    // cppcheck-suppress memleak
}

void g_new_if_test()
{
    struct a {
        int b;
    };

    struct a * pNew3;
    if (pNew3 = g_new(struct a, 6)) {
        printf("%p", pNew3);
    }
    // cppcheck-suppress memleak
}

void g_new0_test()
{
    struct a {
        int b;
    };
    // valid
    struct a * pNew1 = g_new0(struct a, 5);
    printf("%p", pNew1);
    g_free(pNew1);

    // cppcheck-suppress leakReturnValNotUsed
    g_new0(struct a, 1);

    struct a * pNew2 = g_new0(struct a, 2);
    printf("%p", pNew2);
    // cppcheck-suppress memleak
}

void g_try_new_test()
{
    struct a {
        int b;
    };
    // valid
    struct a * pNew1 = g_try_new(struct a, 5);
    printf("%p", pNew1);
    g_free(pNew1);

    // cppcheck-suppress leakReturnValNotUsed
    g_try_new(struct a, 1);

    struct a * pNew2 = g_try_new(struct a, 2);
    printf("%p", pNew2);
    // cppcheck-suppress memleak
}
void g_try_new0_test()
{
    struct a {
        int b;
    };
    // valid
    struct a * pNew1 = g_try_new0(struct a, 5);
    printf("%p", pNew1);
    g_free(pNew1);

    // cppcheck-suppress leakReturnValNotUsed
    g_try_new0(struct a, 1);

    struct a * pNew2 = g_try_new0(struct a, 2);
    printf("%p", pNew2);
    // cppcheck-suppress memleak
}

void g_renew_test()
{
    struct a {
        int b;
    };
    // TODO cppcheck-suppress leakReturnValNotUsed
    g_renew(struct a, NULL, 1);

    struct a * pNew = g_new(struct a, 1);
    pNew = g_renew(struct a, pNew, 2); // No memleakOnRealloc since g_renew aborts if it fails
    printf("%p", pNew);

    // cppcheck-suppress memleak
}

void g_try_renew_test()
{
    struct a {
        int b;
    };
    // TODO cppcheck-suppress leakReturnValNotUsed
    g_try_renew(struct a, NULL, 1);

    struct a * pNew = g_try_new(struct a, 1);
    // TODO cppcheck-suppress memleakOnRealloc
    pNew = g_try_renew(struct a, pNew, 2);
    printf("%p", pNew);

    // cppcheck-suppress memleak
}

void g_error_new_test()
{
    // valid
    GError * pNew1 = g_error_new(1, -2, "a %d", 1);
    printf("%p", pNew1);
    g_error_free(pNew1);

    // cppcheck-suppress ignoredReturnValue
    // cppcheck-suppress leakReturnValNotUsed
    g_error_new(1, -2, "a %d", 1);

    GError * pNew2 = g_error_new(1, -2, "a %d", 1);
    printf("%p", pNew2);
    // cppcheck-suppress memleak
}
