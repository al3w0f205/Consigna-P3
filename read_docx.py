import docx

def read_docx(file_path, output_path):
    doc = docx.Document(file_path)
    with open(output_path, "w", encoding="utf-8") as f:
        for child in doc.element.body:
            if child.tag.endswith('p'):
                p = docx.text.paragraph.Paragraph(child, doc)
                if p.text.strip():
                    f.write(p.text + "\n\n")
            elif child.tag.endswith('tbl'):
                t = docx.table.Table(child, doc)
                f.write("--- TABLA ---\n")
                for row in t.rows:
                    row_text = []
                    for cell in row.cells:
                        row_text.append(cell.text.strip().replace("\n", " "))
                    f.write(" | ".join(row_text) + "\n")
                f.write("-------------\n\n")

if __name__ == "__main__":
    read_docx(r"C:\Users\power\Desktop\ActividadEvaluativa_RC2_ ISWZ1102_202610.docx", "plantilla.txt")
    print("Conversión completada.")
