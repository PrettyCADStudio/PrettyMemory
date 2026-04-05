from __future__ import annotations

import re
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 2:
        return 1

    path = Path(sys.argv[1])
    content = path.read_text(encoding="utf-8-sig")
    content = re.sub(
        r"@~chinese\{([^}]*)\}",
        r'\\htmlonly <span class="doc-lang doc-lang-chinese">\\endhtmlonly \1 \\htmlonly </span>\\endhtmlonly',
        content,
    )
    content = re.sub(
        r"@~english\{([^}]*)\}",
        r'\\htmlonly <span class="doc-lang doc-lang-english">\\endhtmlonly \1 \\htmlonly </span>\\endhtmlonly',
        content,
    )

    sys.stdout.reconfigure(encoding="utf-8")
    sys.stdout.write(content)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
