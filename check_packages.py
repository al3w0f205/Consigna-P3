import sys
for pkg in ['markitdown', 'docx', 'docx2txt', 'pypandoc', 'mammoth']:
    try:
        __import__(pkg)
        print(f"{pkg}: disponible")
    except ImportError:
        print(f"{pkg}: no disponible")
