import PyPDF2

try:
    with open("DAA synopsis (1).pdf", "rb") as file:
        reader = PyPDF2.PdfReader(file)
        text = ""
        for page in reader.pages:
            text += page.extract_text() + "\n"
        with open("pdf_text.txt", "w", encoding="utf-8") as out_file:
            out_file.write(text)
except Exception as e:
    with open("pdf_text.txt", "w", encoding="utf-8") as out_file:
        out_file.write(f"Error: {e}")
