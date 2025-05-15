import React, { useEffect, useState, useContext } from 'react';
import { useNavigate } from 'react-router-dom';
import { UserContext } from '../UserContext';
const BattleLobby = () => {
  const navigate = useNavigate();
  const [waitingMessage, setWaitingMessage] = useState('Waiting for an opponent to join...');
  const [isMatched, setIsMatched] = useState(false);
  const { username } = useContext(UserContext);

  useEffect(() => {
    const interval = setInterval(async () => {
      try {
        const response = await fetch('http://localhost:18080/api/check_match', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json',
            },
            body: JSON.stringify({ username }),
          });
        const data = await response.json();
        

        if (data.ready) {
          clearInterval(interval);
          setIsMatched(true);
          setWaitingMessage('Match found! Starting the game...');
          navigate('/tictak'); // Navigate to the game screen
        }
      } catch (error) {
        console.error('Match check failed:', error);
      }
    }, 2000);

    return () => clearInterval(interval);
  }, [navigate]);

  return (
    <div style={styles.wrapper}>
      <div style={styles.card}>
        <h1 style={styles.heading}>⚔️ Battle Lobby</h1>
        <p style={styles.message}>{waitingMessage}</p>
        {!isMatched && <div style={styles.dots}><span>.</span><span>.</span><span>.</span></div>}
      </div>
    </div>
  );
};

const styles = {
  wrapper: {
    height: '100vh',
    display: 'flex',
    justifyContent: 'center',
    alignItems: 'center',
    background: 'linear-gradient(135deg, #2c3e50, #3498db)',
  },
  card: {
    backgroundColor: '#fff',
    padding: '40px 60px',
    borderRadius: '20px',
    boxShadow: '0 10px 25px rgba(0, 0, 0, 0.2)',
    textAlign: 'center',
    fontFamily: 'Arial, sans-serif',
    color: '#2c3e50',
  },
  heading: {
    fontSize: '2.2rem',
    marginBottom: '20px',
  },
  message: {
    fontSize: '1.2rem',
    marginBottom: '20px',
  },
  dots: {
    fontSize: '2rem',
    animation: 'blink 1.2s infinite',
    display: 'flex',
    justifyContent: 'center',
    gap: '5px',
  },
};

// CSS animation via keyframes injection
const styleSheet = document.styleSheets[0];
const keyframes = `
@keyframes blink {
  0% { opacity: 0.2; }
  50% { opacity: 1; }
  100% { opacity: 0.2; }
}
`;
styleSheet.insertRule(keyframes, styleSheet.cssRules.length);

export default BattleLobby;
