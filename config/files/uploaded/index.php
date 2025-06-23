#!/usr/bin/env php-cgi
<?php
header('Content-Type: text/html; charset=UTF-8');

$requestPath = $_SERVER['REQUEST_URI'];
$directoryPath = __DIR__;

function formatFileTime($mtime) {
    return date('Y-m-d H:i', $mtime);
}

function formatFileSize($bytes) {
    if ($bytes == 0) return '0 B';
    $units = array('B', 'KB', 'MB', 'GB');
    $power = floor(log($bytes, 1024));
    return round($bytes / pow(1024, $power), 2) . ' ' . $units[$power];
}

function calculateParentPath($requestPath) {
    $parentPath = $requestPath;
    if (strlen($parentPath) > 1 && $parentPath[strlen($parentPath) - 1] == '/') {
        $parentPath = substr($parentPath, 0, -1);
    }
    $lastSlash = strrpos($parentPath, '/');
    if ($lastSlash !== false) {
        $parentPath = substr($parentPath, 0, $lastSlash + 1);
    } else {
        $parentPath = "/";
    }
    return $parentPath;
}

function getFileIcon($filename) {
    $ext = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
    $icons = array(
        'pdf' => '📄', 'txt' => '📝', 'doc' => '📄', 'docx' => '📄',
        'jpg' => '🖼️', 'jpeg' => '🖼️', 'png' => '🖼️', 'gif' => '🖼️', 'svg' => '🖼️',
        'mp4' => '🎬', 'avi' => '🎬', 'mov' => '🎬', 'mkv' => '🎬',
        'mp3' => '🎵', 'wav' => '🎵', 'flac' => '🎵',
        'zip' => '📦', 'rar' => '📦', 'tar' => '📦', '7z' => '📦',
        'html' => '🌐', 'htm' => '🌐', 'css' => '🎨', 'js' => '⚡',
        'php' => '🐘', 'py' => '🐍', 'cpp' => '⚙️', 'c' => '⚙️',
        'conf' => '⚙️', 'config' => '⚙️'
    );
    return isset($icons[$ext]) ? $icons[$ext] : '📄';
}

// Read the HTML template and CSS
$template = file_get_contents(__DIR__ . '/template.html');
$css = file_get_contents(__DIR__ . '/kawaii-style.css');

// Generate content
$content = '';

if ($requestPath != "/") {
    $parentPath = calculateParentPath($requestPath);
    $content .= '<div class="file-card parent-dir">
        <div class="file-info">
            <div class="file-icon">📁</div>
            <div class="file-details">
                <h3>📂 Parent Directory</h3>
                <p>Return to previous folder</p>
            </div>
        </div>
        <div class="file-actions">
            <a href="' . htmlspecialchars($parentPath) . '" class="view-btn">🔙 Back</a>
        </div>
    </div>';
}

$files = scandir($directoryPath);
$hasFiles = false;

foreach ($files as $filename) {
    if ($filename[0] == '.' || $filename == 'index.php' || $filename == 'template.html' || $filename == 'kawaii-style.css') continue;
    
    $fullPath = $directoryPath . '/' . $filename;
    $linkPath = rtrim($requestPath, '/') . '/' . $filename;
    if (!file_exists($fullPath)) continue;
    
    $hasFiles = true;
    $filestat = stat($fullPath);
    $timeStr = formatFileTime($filestat['mtime']);
    $icon = getFileIcon($filename);
    
    if (is_dir($fullPath)) {
        $content .= '<div class="file-card">
            <div class="file-info">
                <div class="file-icon">📁</div>
                <div class="file-details">
                    <h3>' . htmlspecialchars($filename) . '</h3>
                    <p>📅 ' . htmlspecialchars($timeStr) . '</p>
                    <p>📂 Directory</p>
                </div>
            </div>
            <div class="file-actions">
                <a href="' . htmlspecialchars($linkPath) . '/" class="view-btn">
                    <img src="/assets/sloth.png" alt="View" class="btn-icon">View
                </a>
            </div>
        </div>';
    } else {
        $content .= '<div class="file-card">
            <div class="file-info">
                <div class="file-icon">' . $icon . '</div>
                <div class="file-details">
                    <h3>' . htmlspecialchars($filename) . '</h3>
                    <p>📅 ' . htmlspecialchars($timeStr) . '</p>
                    <p>📏 ' . formatFileSize($filestat['size']) . '</p>
                </div>
            </div>
            <div class="file-actions">
                <a href="' . htmlspecialchars($linkPath) . '" class="view-btn">
                    <img src="/assets/sloth.png" alt="View" class="btn-icon"> View
                </a>
                <button class="delete-btn" onclick="deleteFile(\'' . htmlspecialchars($filename, ENT_QUOTES) . '\')" title="Delete file">
                Delete <img src="/assets/octopus.png" alt="Delete" class="btn-icon-right">
                </button>
            </div>
        </div>';
    }
}

if (!$hasFiles && $requestPath == "/") {
    $content .= '<div class="empty-state">
        <img src="/assets/sloth.png" alt="Empty" style="width: 80px; height: 80px; opacity: 0.5; margin-bottom: 20px;">
        <h2>No files to display</h2>
        <p>This folder is empty like the heart of a sad sloth</p>
    </div>';
}

// Replace placeholders in template
$html = str_replace('{{PATH}}', htmlspecialchars($requestPath), $template);
$html = str_replace('{{CONTENT}}', $content, $html);
$html = str_replace('<link rel="stylesheet" href="kawaii-style.css">', '<style>' . $css . '</style>', $html);

echo $html;
?>
