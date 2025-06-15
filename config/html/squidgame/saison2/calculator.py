#!/usr/bin/env python3

import cgi
import cgitb
import os
import sys
from datetime import datetime

cgitb.enable()

def escape_html(text):
    if text is None:
        return ""
    text = str(text)
    text = text.replace("&", "&amp;")
    text = text.replace("<", "&lt;")
    text = text.replace(">", "&gt;")
    text = text.replace('"', "&quot;")
    text = text.replace("'", "&#39;")
    return text

def calculate(number1, number2, operation):
    try:
        num1 = float(number1)
        num2 = float(number2)
        if operation == '+':
            return num1 + num2, 'Addition'
        elif operation == '-':
            return num1 - num2, 'Soustraction'
        elif operation == '*':
            return num1 * num2, 'Multiplication'
        elif operation == '/':
            if num2 == 0:
                return None, 'Division par zéro impossible!'
            return num1 / num2, 'Division'
        elif operation == '%':
            if num2 == 0:
                return None, 'Modulo par zéro impossible!'
            return num1 % num2, 'Modulo'
        elif operation == '**':
            return num1 ** num2, 'Puissance'
        else:
            return None, 'Opération non valide!'
    except (ValueError, TypeError):
        return None, 'Nombres non valides!'

def main():
    print("Content-Type: text/html; charset=utf-8")
    print("")
    form = cgi.FieldStorage()
    number1 = form.getvalue("number1", "")
    number2 = form.getvalue("number2", "")
    operation = form.getvalue("operation", "+")
    request_method = os.environ.get('REQUEST_METHOD', 'GET')
    post_data = {
        'number1': number1,
        'number2': number2,
        'operation': operation
    }
    print("""<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Calculette Python - Webserv Test</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            max-width: 500px;
            margin: 50px auto;
            padding: 20px;
            background-color: #f5f5f5;
        }}
        .calculator {{
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }}
        h1 {{
            text-align: center;
            color: #333;
        }}
        .form-group {{
            margin-bottom: 15px;
        }}
        label {{
            display: block;
            margin-bottom: 5px;
            color: #555;
        }}
        input[type="number"], select {{
            width: 100%;
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
        }}
        button {{
            width: 100%;
            padding: 10px;
            background-color: #28a745;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }}
        button:hover {{
            background-color: #218838;
        }}
        .result {{
            margin-top: 20px;
            padding: 15px;
            background-color: #e9ecef;
            border-radius: 4px;
            text-align: center;
        }}
        .error {{
            background-color: #f8d7da;
            color: #721c24;
        }}
        .success {{
            background-color: #d4edda;
            color: #155724;
        }}
        .info {{
            margin-top: 20px;
            text-align: center;
            font-size: 12px;
            color: #666;
            border-top: 1px solid #ddd;
            padding-top: 15px;
        }}
        .debug {{
            background-color: #fff3cd;
            color: #856404;
            padding: 10px;
            border-radius: 4px;
            margin-top: 10px;
            font-size: 11px;
        }}
        .python-badge {{
            background-color: #3776ab;
            color: white;
            padding: 2px 6px;
            border-radius: 3px;
            font-size: 10px;
        }}
    </style>
</head>
<body>
    <div class="calculator">
        <h1>🐍 Calculette Python <span class="python-badge">Python CGI</span></h1>
        
        <form method="POST" action="">
            <div class="form-group">
                <label for="number1">Premier nombre:</label>
                <input type="number" step="any" id="number1" name="number1" value="{}" required>
            </div>
            
            <div class="form-group">
                <label for="operation">Opération:</label>
                <select id="operation" name="operation" required>
                    <option value="">Sélectionnez une opération</option>
                    <option value="+" {}>Addition (+)</option>
                    <option value="-" {}>Soustraction (-)</option>
                    <option value="*" {}>Multiplication (×)</option>
                    <option value="/" {}>Division (÷)</option>
                    <option value="%" {}>Modulo (%)</option>
                    <option value="**" {}>Puissance (^)</option>
                </select>
            </div>
            
            <div class="form-group">
                <label for="number2">Deuxième nombre:</label>
                <input type="number" step="any" id="number2" name="number2" value="{}" required>
            </div>
            
            <button type="submit">Calculer</button>
        </form>""".format(
        escape_html(number1),
        'selected' if operation == '+' else '',
        'selected' if operation == '-' else '',
        'selected' if operation == '*' else '',
        'selected' if operation == '/' else '',
        'selected' if operation == '%' else '',
        'selected' if operation == '**' else '',
        escape_html(number2)
    ))
    print("""
        <div class="debug">
            <strong>Debug Info:</strong><br>
            REQUEST_METHOD: {}<br>
            CONTENT_TYPE: {}<br>
            CONTENT_LENGTH: {}<br>
            POST vars count: {}<br>
            POST data: {}
        </div>""".format(
        escape_html(os.environ.get('REQUEST_METHOD', 'UNDEFINED')),
        escape_html(os.environ.get('CONTENT_TYPE', 'UNDEFINED')),
        escape_html(os.environ.get('CONTENT_LENGTH', 'UNDEFINED')),
        len(post_data),
        ' '.join('{}={}'.format(k, v) for k, v in post_data.items()) if post_data else 'EMPTY'
    ))
    if request_method == 'POST':
        if number1 and number2 and operation:
            result, operation_text = calculate(number1, number2, operation)
            if result is not None:
                print("""
        <div class="result success">
            <strong>Résultat de la {}:</strong><br>
            <span style="font-size: 18px;">{} {} {} = <strong>{}</strong></span>
        </div>""".format(
                    operation_text,
                    escape_html(number1),
                    escape_html(operation),
                    escape_html(number2),
                    result
                ))
            else:
                print("""
        <div class="result error">
            <strong>Erreur:</strong> {}
        </div>""".format(operation_text))
        else:
            print("""
        <div class="result error">
            <strong>Erreur:</strong> Veuillez remplir tous les champs avec des nombres valides!
        </div>""")
    print("""
        <div class="info">
            <p><strong>Test CGI Python pour Webserv</strong></p>
            <p>Timestamp: {}</p>
            <p>Méthode: {}</p>
            <p>Serveur: {}</p>
            <p>Python Version: {}</p>
        </div>
    </div>
</body>
</html>""".format(
        datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        escape_html(request_method),
        escape_html(os.environ.get('SERVER_SOFTWARE', 'Webserv/1.0')),
        escape_html(sys.version.split()[0])
    ))

if __name__ == '__main__':
    main()
