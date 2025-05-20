import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import { UserContext } from '../UserContext';
import { useContext } from 'react';

const LoginSignup = () => {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [message, setMessage] = useState("");
  const navigate = useNavigate();
    // inside your login function, after login success:
    const { setUsername: setUsernameContext } = useContext(UserContext);


  const handleRequest = async (endpoint) => {
    try {
      const response = await fetch(`${import.meta.env.VITE_API_URL}/api/${endpoint}`, {

        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ username, password }),
      });
      console.log(response);
      
      const data =  response.json();
    //   console.log(data);
      setMessage(data.message);
      
      if (response.ok) {
        // Save login state (could be session/local storage later)
        console.log("navigating to lobby", { username });
        setUsernameContext(username); // Set the username in context
        navigate("/lobby");
      }
    } catch (error) {
      console.error("Error:", error);
      setMessage("Something went wrong.");
    }
  };
  

  return (
    <div style={styles.container}>
      <h1 style={styles.title}>Welcome to Games Arena</h1>

      <div style={styles.box}>
        <input
          style={styles.input}
          type="text"
          placeholder="Username"
          value={username}
          onChange={(e) => setUsername(e.target.value)}
        />

        <input
          style={styles.input}
          type="password"
          placeholder="Password"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
        />

        <button style={styles.button} onClick={() => handleRequest("login")}>
          Log In
        </button>
        <button style={styles.button} onClick={() => handleRequest("signup")}>
          Sign Up
        </button>

        <p>{message}</p>
      </div>
    </div>
  );
};

const styles = {
  container: {
    marginTop: "80px",
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
  },
  title: {
    fontSize: "36px",
    marginBottom: "20px",
  },
  box: {
    border: "1px solid #ccc",
    borderRadius: "10px",
    padding: "20px",
    width: "300px",
    textAlign: "center",
    boxShadow: "0px 2px 8px rgba(0,0,0,0.1)",
  },
  input: {
    width: "100%",
    padding: "10px",
    margin: "10px 0",
    fontSize: "16px",
  },
  button: {
    width: "100%",
    padding: "10px",
    marginTop: "10px",
    fontSize: "16px",
    cursor: "pointer",
  },
};

export default LoginSignup;
