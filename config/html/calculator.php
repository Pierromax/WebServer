<?php
// Test CGI pour Webserv
?>
<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Calculette PHP - Webserv Test</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 500px;
            margin: 50px auto;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .calculator {
            background: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            text-align: center;
            color: #333;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #555;
        }
        input[type="number"], select {
            width: 100%;
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            box-sizing: border-box;
        }
        button {
            width: 100%;
            padding: 10px;
            background-color: #007bff;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        button:hover {
            background-color: #0056b3;
        }
        .result {
            margin-top: 20px;
            padding: 15px;
            background-color: #e9ecef;
            border-radius: 4px;
            text-align: center;
        }
        .error {
            background-color: #f8d7da;
            color: #721c24;
        }
        .success {
            background-color: #d4edda;
            color: #155724;
        }
        .info {
            margin-top: 20px;
            text-align: center;
            font-size: 12px;
            color: #666;
            border-top: 1px solid #ddd;
            padding-top: 15px;
        }
        .debug {
            background-color: #fff3cd;
            color: #856404;
            padding: 10px;
            border-radius: 4px;
            margin-top: 10px;
            font-size: 11px;
        }
    </style>
</head>
<body>
    <div class="calculator">
        <h1>🧮 Calculette PHP</h1>
        
        <form method="POST" action="">
            <div class="form-group">
                <label for="number1">Premier nombre:</label>
                <input type="number" step="any" id="number1" name="number1" value="<?php echo isset($_POST['number1']) ? htmlspecialchars($_POST['number1']) : ''; ?>" required>
            </div>
            
            <div class="form-group">
                <label for="operation">Opération:</label>
                <select id="operation" name="operation" required>
                    <option value="">Sélectionnez une opération</option>
                    <option value="+" <?php echo (isset($_POST['operation']) && $_POST['operation'] == '+') ? 'selected' : ''; ?>>Addition (+)</option>
                    <option value="-" <?php echo (isset($_POST['operation']) && $_POST['operation'] == '-') ? 'selected' : ''; ?>>Soustraction (-)</option>
                    <option value="*" <?php echo (isset($_POST['operation']) && $_POST['operation'] == '*') ? 'selected' : ''; ?>>Multiplication (×)</option>
                    <option value="/" <?php echo (isset($_POST['operation']) && $_POST['operation'] == '/') ? 'selected' : ''; ?>>Division (÷)</option>
                    <option value="%" <?php echo (isset($_POST['operation']) && $_POST['operation'] == '%') ? 'selected' : ''; ?>>Modulo (%)</option>
                    <option value="**" <?php echo (isset($_POST['operation']) && $_POST['operation'] == '**') ? 'selected' : ''; ?>>Puissance (^)</option>
                </select>
            </div>
            
            <div class="form-group">
                <label for="number2">Deuxième nombre:</label>
                <input type="number" step="any" id="number2" name="number2" value="<?php echo isset($_POST['number2']) ? htmlspecialchars($_POST['number2']) : ''; ?>" required>
            </div>
            
            <button type="submit">Calculer</button>
        </form>

        <?php
        // Debug info
        echo '<div class="debug">';
        echo '<strong>Debug Info:</strong><br>';
        echo 'REQUEST_METHOD: ' . (isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : 'UNDEFINED') . '<br>';
        echo 'Content-Type: ' . (isset($_SERVER['CONTENT_TYPE']) ? $_SERVER['CONTENT_TYPE'] : 'UNDEFINED') . '<br>';
        echo 'Content-Length: ' . (isset($_SERVER['CONTENT_LENGTH']) ? $_SERVER['CONTENT_LENGTH'] : 'UNDEFINED') . '<br>';
        echo 'POST vars count: ' . count($_POST) . '<br>';
        echo 'POST data: ';
        if (!empty($_POST)) {
            foreach ($_POST as $key => $value) {
                echo $key . '=' . $value . ' ';
            }
        } else {
            echo 'EMPTY';
        }
        echo '<br>';
        
        // Afficher les données brutes reçues
        $raw_input = file_get_contents('php://input');
        echo 'Raw input: ' . htmlspecialchars($raw_input) . '<br>';
        echo '</div>';
        
        if ($_SERVER['REQUEST_METHOD'] === 'POST') {
            // Vérifier si on a bien reçu les données POST
            if (isset($_POST['number1']) && isset($_POST['number2']) && isset($_POST['operation'])) {
                $number1 = $_POST['number1'];
                $number2 = $_POST['number2'];
                $operation = $_POST['operation'];
                
                if (is_numeric($number1) && is_numeric($number2) && !empty($operation)) {
                    $result = null;
                    $error = false;
                    $operation_text = '';
                    
                    switch ($operation) {
                        case '+':
                            $result = $number1 + $number2;
                            $operation_text = 'Addition';
                            break;
                        case '-':
                            $result = $number1 - $number2;
                            $operation_text = 'Soustraction';
                            break;
                        case '*':
                            $result = $number1 * $number2;
                            $operation_text = 'Multiplication';
                            break;
                        case '/':
                            if ($number2 == 0) {
                                echo '<div class="result error"><strong>Erreur:</strong> Division par zéro impossible!</div>';
                                $error = true;
                            } else {
                                $result = $number1 / $number2;
                                $operation_text = 'Division';
                            }
                            break;
                        case '%':
                            if ($number2 == 0) {
                                echo '<div class="result error"><strong>Erreur:</strong> Modulo par zéro impossible!</div>';
                                $error = true;
                            } else {
                                $result = $number1 % $number2;
                                $operation_text = 'Modulo';
                            }
                            break;
                        case '**':
                            $result = pow($number1, $number2);
                            $operation_text = 'Puissance';
                            break;
                        default:
                            echo '<div class="result error"><strong>Erreur:</strong> Opération non valide!</div>';
                            $error = true;
                    }
                    
                    if (!$error && $result !== null) {
                        echo '<div class="result success">';
                        echo '<strong>Résultat de la ' . $operation_text . ':</strong><br>';
                        echo '<span style="font-size: 18px;">' . $number1 . ' ' . $operation . ' ' . $number2 . ' = <strong>' . $result . '</strong></span>';
                        echo '</div>';
                    }
                } else {
                    echo '<div class="result error"><strong>Erreur:</strong> Veuillez remplir tous les champs avec des nombres valides!</div>';
                }
            } else {
                echo '<div class="result error"><strong>Erreur:</strong> Données POST manquantes! Vérifiez que le formulaire a été correctement soumis.</div>';
            }
        }
        ?>
        
        <div class="info">
            <p><strong>Test CGI pour Webserv</strong></p>
            <p>Timestamp: <?php echo date('Y-m-d H:i:s'); ?></p>
            <p>Méthode: <?php echo isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : 'UNDEFINED'; ?></p>
            <?php if (isset($_SERVER['SERVER_SOFTWARE'])): ?>
                <p>Serveur: <?php echo $_SERVER['SERVER_SOFTWARE']; ?></p>
            <?php else: ?>
                <p>Serveur: Webserv/1.0</p>
            <?php endif; ?>
        </div>
    </div>
</body>
</html>