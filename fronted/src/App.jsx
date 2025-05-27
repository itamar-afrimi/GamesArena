import React from "react";
import { BrowserRouter as Router, Routes, Route } from "react-router-dom";
import LoginSignup from "./compoments/LoginSignup";
import Lobby from "./compoments/Lobby";
import Tictac from "./compoments/Tictac";
import BattleLobby from "./compoments/BattleLobby"; // Import the BattleLobby component
import Backgammon from "./compoments/Backgammon";



function App() {
  return (
       
    <Router>
      <Routes>
        <Route path="/" element={<LoginSignup />} />
        <Route path="/lobby" element={<Lobby />} />
        <Route  path="/tictak" element={<Tictac />} /> 
        <Route path="/battle_lobby" element={<BattleLobby />} /> {/* Redirect to LoginSignup for any other route */}  
        <Route path="/backgammon" element={<Backgammon />} /> {/* Add the Backgammon route */}
      </Routes>
    </Router>
  );
}

export default App;
