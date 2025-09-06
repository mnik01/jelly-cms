# Jelly CMS

A fast, lightweight static site generator written in C that supports multilingual content, template partials, and asset management.

## Features

- **🌍 Multilingual Support**: Automatic locale detection and site generation
- **🧩 Template Partials**: Reusable HTML components with includes
- **📁 Asset Management**: Automatic copying of vendor, public, and asset files
- **⚡ Fast Build Times**: Native C implementation for maximum performance
- **🎯 Simple Syntax**: Minimal templating with clear syntax
- **📦 Zero Dependencies**: Single executable with no external requirements
- TODO: add markdown support for content

## Quick Start

### Installation

1. Clone or download the project files (`main.c` and `Makefile`)
2. Build the executable:
   ```bash
   make
   ```

### Project Structure

```
your-project/
├── assets/                 # Static assets (images, fonts, etc.)
│   └── images/
│       └── logo.png
├── locale/                 # Locale files (optional)
│   ├── en.json
│   ├── ru.json
│   └── kk.json
├── public/                 # Public files copied as-is
│   ├── robots.txt
│   ├── sitemap.xml
│   └── main.css
├── src/
│   ├── pages/              # Your HTML pages
│   │   ├── index.html
│   │   └── about/
│   │       └── contacts.html
│   └── partials/           # Reusable components
│       ├── header.html
│       └── footer.html
├── vendor/                 # Third-party libraries
│   └── alpinejs.min.js
└── jelly-cms              # The built executable
```

### Build Your Site

```bash
./jelly-cms build
```

This creates a `build/` directory with your generated site.

## Templating

### Including Partials

Use the include syntax to embed reusable components:

```html
<!DOCTYPE html>
<html>
<head>
    <title>My Site</title>
</head>
<body>
    <!-- %include.header% -->
    
    <main>
        <h1>Welcome to my site</h1>
    </main>
    
    <!-- %include.footer% -->
</body>
</html>
```

### Locale Variables

Reference locale-specific content using the locale syntax:

```html
<h1>%locale.welcome_title%</h1>
<p>Contact us: %locale.phone%</p>
<address>%locale.address%</address>
```

## Localization

### Setting Up Locales

1. Create a `locale/` directory
2. Add JSON files for each language (e.g., `en.json`, `ru.json`, `kk.json`)

**Example `locale/en.json`:**
```json
{
  "welcome_title": "Welcome to Our Website",
  "phone": "+1 555 123 4567",
  "address": "123 Main St, City, Country",
  "nav_home": "Home",
  "nav_about": "About",
  "footer_copyright": "© 2024 My Company"
}
```

**Example `locale/ru.json`:**
```json
{
  "welcome_title": "Добро пожаловать на наш сайт",
  "phone": "+7 123 456 7890",
  "address": "ул. Главная 123, Город, Страна",
  "nav_home": "Главная",
  "nav_about": "О нас",
  "footer_copyright": "© 2024 Моя Компания"
}
```

### Build Output

With locales, Jelly CMS generates:
```
build/
├── en/
│   ├── index.html
│   └── about/
│       └── contacts.html
├── ru/
│   ├── index.html
│   └── about/
│       └── contacts.html
├── assets/
│   └── images/
│       └── logo.png
├── vendor/
│   └── alpinejs.min.js
├── robots.txt
├── sitemap.xml
└── main.css
```

Without locales:
```
build/
├── index.html
├── about/
│   └── contacts.html
├── assets/
├── vendor/
├── robots.txt
├── sitemap.xml
└── main.css
```

## Directory Behavior

| Source Directory | Build Behavior |
|-----------------|----------------|
| `assets/` | Copied to `build/assets/` |
| `public/` | Contents copied directly to `build/` root |
| `vendor/` | Copied to `build/vendor/` |
| `src/pages/` | Processed as templates, structure preserved |
| `src/partials/` | Used for includes, not copied to build |
| `locale/` | Used for templating, not copied to build |

## Example Templates

### Main Page (`src/pages/index.html`)
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>%locale.site_title%</title>
    <link rel="stylesheet" href="/main.css">
</head>
<body>
    <!-- %include.header% -->
    
    <main>
        <h1>%locale.welcome_title%</h1>
        <p>%locale.welcome_message%</p>
    </main>
    
    <!-- %include.footer% -->
    <script src="/vendor/alpinejs.min.js"></script>
</body>
</html>
```

### Header Partial (`src/partials/header.html`)
```html
<header>
    <nav>
        <img src="/assets/images/logo.png" alt="Logo">
        <ul>
            <li><a href="/">%locale.nav_home%</a></li>
            <li><a href="/about/">%locale.nav_about%</a></li>
        </ul>
    </nav>
</header>
```

### Footer Partial (`src/partials/footer.html`)
```html
<footer>
    <p>%locale.footer_copyright%</p>
    <p>%locale.contact_info%</p>
</footer>
```

## Build Commands

### Basic Commands
```bash
# Build the site
./jelly-cms build

# Clean build artifacts
make clean

# Rebuild everything
make clean && make && ./jelly-cms build
```

### Make Targets
```bash
make              # Build jelly-cms executable
make clean        # Remove build artifacts
make install      # Install to /usr/local/bin (requires sudo)
make uninstall    # Remove from /usr/local/bin (requires sudo)
make test         # Build and run test
make help         # Show available targets
```

## Advanced Usage

### Nested Partials
Partials can include other partials:

**`src/partials/layout.html`:**
```html
<!DOCTYPE html>
<html>
<head>
    <!-- %include.meta% -->
</head>
<body>
    <!-- %include.header% -->
    <main>
        <!-- Content will be here -->
    </main>
    <!-- %include.footer% -->
</body>
</html>
```

### Complex Directory Structures
```
src/pages/
├── index.html
├── blog/
│   ├── index.html
│   └── posts/
│       ├── first-post.html
│       └── second-post.html
└── products/
    ├── index.html
    └── category/
        └── item.html
```

This structure is preserved in the build output for each locale.

## Troubleshooting

### Common Issues

**Segmentation Fault:**
- Check that all referenced partials exist in `src/partials/`
- Ensure locale JSON files are valid JSON
- Verify file permissions for all source directories

**Missing Files in Build:**
- Ensure source directories exist (`src/pages/`, etc.)
- Check file paths are correct (case-sensitive)
- Verify JSON syntax in locale files

**Locale Variables Not Replaced:**
- Check JSON syntax in locale files
- Ensure locale keys match exactly (case-sensitive)
- Verify locale files are in `locale/` directory

### Debug Mode
Run with verbose output to see detailed processing:
```bash
./jelly-cms build 2>&1 | tee build.log
```

### File Size Limits
- Maximum file size: 64KB per template
- Maximum locale entries: 100 per language
- Maximum locales: 10

## Performance

- **Build Speed**: Processes hundreds of pages in milliseconds
- **Memory Usage**: Minimal RAM footprint (~1-2MB)
- **File Size**: Single ~50KB executable
- **Dependencies**: None (statically linked)

## License

This project is provided as-is. Feel free to modify and distribute according to your needs.

## Contributing

Since this is a simple C program, contributions are welcome:
1. Fork the project
2. Make your changes to `main.c`
3. Test thoroughly
4. Submit a pull request

For bug reports or feature requests, please provide:
- Your project structure
- Locale files (if using multilingual features)
- Error messages or unexpected behavior
- Operating system and compiler version
