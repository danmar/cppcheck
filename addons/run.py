
import cppcheckdata, runpy, sys, os

__checkers__ = []

def checker(f):
    __checkers__.append(f)
    return f


__errorid__ = ''
__addon_name__ = ''
def reportError(location, severity, message, errorId=None):
    cppcheckdata.reportError(location, severity, message, __addon_name__, errorId or __errorid__)

if __name__ == '__main__':
    sys.modules['cppcheck'] = sys.modules['__main__']
    addon = sys.argv[1]
    parser = cppcheckdata.ArgumentParser()
    args = parser.parse_args(args=sys.argv[2:])

    __addon_name__ = os.path.splitext(os.path.basename(addon))[0]

    runpy.run_path(addon)

    for dumpfile in args.dumpfile:
        if not args.quiet:
            print('Checking %s...' % dumpfile)

        data = cppcheckdata.CppcheckData(dumpfile)

        for cfg in data.iterconfigurations():
            if not args.quiet:
                print('Checking %s, config %s...' % (dumpfile, cfg.name))
            for c in __checkers__:
                __errorid__ = c.__name__
                c(cfg, data)

    sys.exit(cppcheckdata.EXIT_CODE)

