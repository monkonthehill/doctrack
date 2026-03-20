#!/bin/bash

echo "🔧 Setting up DocTrack..."

# Check Homebrew (Mac)
if ! command -v brew &>/dev/null; then
    echo "❌ Homebrew not found. Install it first: https://brew.sh/"
    exit 1
fi

# Install pandoc if not present
if ! command -v pandoc &>/dev/null; then
    echo "📦 Installing pandoc..."
    brew install pandoc
else
    echo "✅ pandoc already installed"
fi

# Optional: install basictex (lighter LaTeX)
if ! command -v pdflatex &>/dev/null; then
    echo "📦 Installing BasicTeX (for PDF support)..."
    brew install --cask basictex
else
    echo "✅ LaTeX already installed"
fi

echo "✅ Setup complete!"
