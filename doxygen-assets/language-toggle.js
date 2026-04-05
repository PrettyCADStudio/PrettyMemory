(function () {
    function preferredLanguage() {
        var stored = window.localStorage.getItem('prettymemory-doc-language');
        if (stored === 'chinese' || stored === 'english') {
            return stored;
        }

        return navigator.language && navigator.language.toLowerCase().indexOf('zh') === 0
            ? 'chinese'
            : 'english';
    }

    function applyLanguage(language) {
        var root = document.documentElement;
        root.classList.remove('doc-language-chinese', 'doc-language-english');
        root.classList.add('doc-language-' + language);
        window.localStorage.setItem('prettymemory-doc-language', language);

        document.querySelectorAll('.doc-language-button').forEach(function (button) {
            button.classList.toggle('is-active', button.getAttribute('data-doc-language') === language);
        });
    }

    document.addEventListener('DOMContentLoaded', function () {
        document.querySelectorAll('.doc-language-button').forEach(function (button) {
            button.addEventListener('click', function () {
                applyLanguage(button.getAttribute('data-doc-language'));
            });
        });

        applyLanguage(preferredLanguage());
    });
}());
