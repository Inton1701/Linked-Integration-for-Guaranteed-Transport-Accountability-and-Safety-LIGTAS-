/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{vue,js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        primary: '#005145',      // Dark teal
        secondary: '#005145',    // Dark teal
        accent: '#005145',       // Dark teal
        dark: '#003d33',         // Darker teal (for text on light backgrounds)
        light: '#ffffff',        // White
      }
    },
  },
  plugins: [],
}
